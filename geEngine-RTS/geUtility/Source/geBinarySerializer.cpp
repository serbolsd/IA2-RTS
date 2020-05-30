/*****************************************************************************/
/**
* @file    geBinarySerializer.cpp
* @author  Samuel Prince (samuel.prince.quezada@gmail.com)
* @date    2017/11/03
* @brief   Encodes all the fields of the provided object into a binary format.
*
* Encodes all the fields of the provided object into a binary format. Fields
* are encoded using their unique IDs.
* Encoded data will remain compatible for decoding even if you modify the
* encoded class, as long as you assign new unique field IDs to added/modified
* fields.
*
* Like for any serializable class, fields are defined in RTTIType that each
* IReflectable class must be able to return.
*
* Any data the object or its children are pointing to will also be serialized
* (unless the pointer isn't registered in RTTIType). Upon decoding the pointer
* addresses will be set to proper values.
*
* @note Child elements are guaranteed to be fully deserialized before their
* parents, except for fields marked with WeakRef flag.
*
* @bug     No known bugs.
*/
/*****************************************************************************/

/*****************************************************************************/
/**
* Includes
*/
/*****************************************************************************/
#include "geBinarySerializer.h"
#include "geException.h"
#include "geDebug.h"
#include "geIReflectable.h"
#include "geRTTIType.h"
#include "geRTTIField.h"
#include "geRTTIPlainField.h"
#include "geRTTIReflectableField.h"
#include "geRTTIReflectablePtrField.h"
#include "geRTTIManagedDataBlockField.h"
#include "geMemorySerializer.h"
#include "geDataStream.h"

#include <unordered_set>

/**
 * @brief A macro that represents a block of code that gets used a lot inside
 *        encodeInternal. It checks if the buffer has enough space, and if it
 *        does it copies the data from the specified location and increments
 *        the needed pointers and counters. If there is not enough space the
 *        buffer is flushed (hopefully to make some space). If there is still
 *        not enough space the entire encoding process ends.
 * @param dataPtr Pointer to data which to copy.
 * @param size    Size of the data to copy
 */
#define COPY_TO_BUFFER(dataIter, size)                                        \
if((*bytesWritten + size) > bufferLength) {                                   \
	m_totalBytesWritten += *bytesWritten;                                       \
	buffer = flushBufferCallback(buffer - *bytesWritten,                        \
                               *bytesWritten,                                 \
                               bufferLength);                                 \
  if (nullptr == buffer || bufferLength < size) {                             \
    return nullptr;                                                           \
  }                                                                           \
	*bytesWritten = 0;                                                          \
}                                                                             \
                                                                              \
memcpy(buffer, dataIter, size);                                               \
buffer += size;                                                               \
*bytesWritten += size;

namespace geEngineSDK {
  using std::function;
  using std::make_pair;
  using std::static_pointer_cast;

  void
  BinarySerializer::encode(IReflectable* object,
                           uint8* buffer,
                           uint32 bufferLength,
                           uint32* bytesWritten,
                           function<uint8*(uint8*, uint32, uint32&)> flushBufferCallback,
                           bool shallow,
                           const UnorderedMap<String, uint64>& params) {
    m_objectsToEncode.clear();
    m_objectAddrToId.clear();
    m_lastUsedObjectId = 1;
    *bytesWritten = 0;
    m_totalBytesWritten = 0;
    m_params = params;

    Vector<SPtr<IReflectable>> encodedObjects;
    uint32 objectId = findOrCreatePersistentId(object);

    // Encode primary object and its value types
    buffer = encodeEntry(object,
                         objectId,
                         buffer,
                         bufferLength,
                         bytesWritten,
                         flushBufferCallback,
                         shallow);

    if (nullptr == buffer) {
      GE_EXCEPT(InternalErrorException,
                "Destination buffer is null or not large enough.");
    }

    //Encode pointed to objects and their value types
    UnorderedSet<uint32> serializedObjects;
    while (true) {
      auto iter = m_objectsToEncode.begin();
      bool foundObjectToProcess = false;
      for (; iter != m_objectsToEncode.end(); ++iter) {
        auto foundExisting = serializedObjects.find(iter->objectId);
        if (serializedObjects.end() != foundExisting){
          continue; //Already processed
        }

        SPtr<IReflectable> curObject = iter->object;
        uint32 curObjectid = iter->objectId;
        serializedObjects.insert(curObjectid);
        m_objectsToEncode.erase(iter);

        buffer = encodeEntry(curObject.get(),
                             curObjectid,
                             buffer,
                             bufferLength,
                             bytesWritten,
                             flushBufferCallback,
                             shallow);
        if (nullptr == buffer) {
          GE_EXCEPT(InternalErrorException,
                    "Destination buffer is null or not large enough.");
        }

        foundObjectToProcess = true;

        //Ensure we keep a reference to the object so it isn't released.
        //The system assigns unique IDs to IReflectable objects based on
        //pointer addresses but if objects get released then same address could
        //be assigned twice.
        //NOTE: To get around this I could assign unique IDs to IReflectable
        //objects
        encodedObjects.push_back(curObject);

        //Need to start over as m_objectsToSerialize was possibly modified
        break;
      }

      if (!foundObjectToProcess) { //We're done
        break;
      }
    }

    //Final flush
    if (*bytesWritten > 0) {
      m_totalBytesWritten += *bytesWritten;
      buffer = flushBufferCallback(buffer - *bytesWritten, *bytesWritten, bufferLength);
    }

    *bytesWritten = m_totalBytesWritten;

    encodedObjects.clear();
    m_objectsToEncode.clear();
    m_objectAddrToId.clear();
  }

  SPtr<IReflectable>
  BinarySerializer::decode(const SPtr<DataStream>& data,
                           uint32 dataLength,
                           const UnorderedMap<String, uint64>& params) {
    m_params = params;

    if (0 == dataLength) {
      return nullptr;
    }

    SPtr<SerializedObject> intermediateObject = _decodeToIntermediate(data, dataLength);
    if (nullptr == intermediateObject) {
      return nullptr;
    }

    return _decodeFromIntermediate(intermediateObject);
  }

  SPtr<IReflectable>
  BinarySerializer::_decodeFromIntermediate(const SPtr<SerializedObject>& serializedObject) {
    m_objectMap.clear();

    SPtr<IReflectable> output;
    RTTITypeBase* type = IReflectable::_getRTTIfromTypeId(serializedObject->getRootTypeId());
    if (nullptr != type) {
      output = type->newRTTIObject();
      auto iterNewObj = m_objectMap.insert(make_pair(serializedObject,
                                                     ObjectToDecode(output,
                                                                    serializedObject)));

      iterNewObj.first->second.decodeInProgress = true;
      decodeEntry(output, serializedObject);
      iterNewObj.first->second.decodeInProgress = false;
      iterNewObj.first->second.isDecoded = true;
    }

    //Go through the remaining objects (should be only ones with weak refs)
    for (auto& iter : m_objectMap) {
      ObjectToDecode& objToDecode = iter.second;

      if (objToDecode.isDecoded) {
        continue;
      }

      objToDecode.decodeInProgress = true;
      decodeEntry(objToDecode.object, objToDecode.serializedObject);
      objToDecode.decodeInProgress = false;
      objToDecode.isDecoded = true;
    }

    m_objectMap.clear();
    return output;
  }

  SPtr<SerializedObject>
  BinarySerializer::_encodeToIntermediate(IReflectable* object, bool shallow) {
    //TODO: This is a hacky way of generating an intermediate format to save
    //development time and complexity. It is hacky because it requires a full
    //on encode to binary and then decode into intermediate. It should be
    //better to modify encoding process so it outputs the intermediate format
    //directly (similar to how decoding works). 
    //This also means that once you have an intermediate format you cannot use
    //it to encode to binary.

    function<void*(SIZE_T)> allocator = &MemoryAllocator<GenAlloc>::allocate;

    MemorySerializer ms;
    uint32 dataLength = 0;
    uint8* data = ms.encode(object, dataLength, allocator, shallow);

    SPtr<MemoryDataStream>
      stream = ge_shared_ptr_new<MemoryDataStream>(data, static_cast<SIZE_T>(dataLength));

    BinarySerializer bs;
    SPtr<SerializedObject> obj = bs._decodeToIntermediate(stream, dataLength, true);

    return obj;
  }

  SPtr<SerializedObject>
  BinarySerializer::_decodeToIntermediate(const SPtr<DataStream>& data,
                                          uint32 dataLength,
                                          bool copyData) {
    bool streamDataBlock = false;
    if (!copyData && data->isFile()) {
      copyData = true;
      streamDataBlock = true;
    }

    uint32 bytesRead = 0;
    m_interimObjectMap.clear();

    SPtr<SerializedObject> rootObj;
    bool hasMore = decodeEntry(data,
                               dataLength,
                               bytesRead,
                               rootObj,
                               copyData,
                               streamDataBlock);
    while (hasMore) {
      SPtr<SerializedObject> dummyObj;
      hasMore = decodeEntry(data,
                            dataLength,
                            bytesRead,
                            dummyObj,
                            copyData,
                            streamDataBlock);
    }

    return rootObj;
  }

  uint8*
  BinarySerializer::encodeEntry(IReflectable* object,
                                uint32 objectId,
                                uint8* buffer,
                                uint32& bufferLength,
                                uint32* bytesWritten,
                                function<uint8*(uint8*, uint32, uint32&)> flushBufferCallback,
                                bool shallow) {
    RTTITypeBase* si = object->getRTTI();
    bool isBaseClass = false;

    //If an object has base classes, we need to iterate through all of them
    do {
      si->onSerializationStarted(object, m_params);

      //Encode object ID & type
      ObjectMetaData objectMetaData = encodeObjectMetaData(objectId,
                                                           si->getRTTIId(),
                                                           isBaseClass);
      COPY_TO_BUFFER(&objectMetaData, sizeof(ObjectMetaData));

      uint32 numFields = si->getNumFields();
      for (uint32 i = 0; i < numFields; ++i) {
        RTTIField* curGenericField = si->getField(i);

        //Copy field ID & other meta-data like field size and type
        uint32 metaData = encodeFieldMetaData(curGenericField->m_uniqueId,
                                           static_cast<uint8>(curGenericField->getTypeSize()),
                                           curGenericField->m_isVectorType,
                                           curGenericField->m_type,
                                           curGenericField->hasDynamicSize(),
                                           false);
        COPY_TO_BUFFER(&metaData, META_SIZE);

        if (curGenericField->m_isVectorType) {
          uint32 arrayNumElems = curGenericField->getArraySize(object);

          // Copy num vector elements
          COPY_TO_BUFFER(&arrayNumElems, NUM_ELEM_FIELD_SIZE);

          switch (curGenericField->m_type)
          {
            case SERIALIZABLE_FIELD_TYPE::kReflectablePtr:
            {
              RTTIReflectablePtrFieldBase*
                curField = static_cast<RTTIReflectablePtrFieldBase*>(curGenericField);

              for (uint32 arrIdx = 0; arrIdx < arrayNumElems; ++arrIdx) {
                SPtr<IReflectable> childObject;

                if (!shallow) {
                  childObject = curField->getArrayValue(object, arrIdx);
                }

                uint32 objId = registerObjectPtr(childObject);
                COPY_TO_BUFFER(&objId, sizeof(uint32));
              }

              break;
            }
            case SERIALIZABLE_FIELD_TYPE::kReflectable:
            {
              RTTIReflectableFieldBase*
                curField = static_cast<RTTIReflectableFieldBase*>(curGenericField);

              for (uint32 arrIdx = 0; arrIdx < arrayNumElems; ++arrIdx) {
                IReflectable& childObject = curField->getArrayValue(object, arrIdx);

                buffer = complexTypeToBuffer(&childObject,
                                             buffer,
                                             bufferLength,
                                             bytesWritten,
                                             flushBufferCallback,
                                             shallow);
                if (nullptr == buffer) {
                  si->onSerializationEnded(object, m_params);
                  return nullptr;
                }
              }
              break;
            }
            case SERIALIZABLE_FIELD_TYPE::kPlain:
            {
              RTTIPlainFieldBase*
                curField = static_cast<RTTIPlainFieldBase*>(curGenericField);

              for (uint32 arrIdx = 0; arrIdx < arrayNumElems; ++arrIdx) {
                uint32 typeSize = 0;
                if (curField->hasDynamicSize()) {
                  typeSize = curField->getArrayElemDynamicSize(object, arrIdx);
                }
                else {
                  typeSize = curField->getTypeSize();
                }

                if ((*bytesWritten + typeSize) > bufferLength) {
                  uint8* tempBuffer = 
                    reinterpret_cast<uint8*>(ge_stack_alloc(static_cast<SIZE_T>(typeSize)));
                  curField->arrayElemToBuffer(object, arrIdx, tempBuffer);

                  buffer = dataBlockToBuffer(tempBuffer,
                                             typeSize,
                                             buffer,
                                             bufferLength,
                                             bytesWritten,
                                             flushBufferCallback);
                  ge_stack_free(tempBuffer);

                  if (nullptr == buffer || 0 == bufferLength) {
                    si->onSerializationEnded(object, m_params);
                    return nullptr;
                  }
                }
                else {
                  curField->arrayElemToBuffer(object, arrIdx, buffer);
                  buffer += typeSize;
                  *bytesWritten += typeSize;
                }
              }
              break;
            }
            default:
              GE_EXCEPT(InternalErrorException,
                        "Error encoding data. Encountered a type I don't know "
                        "how to encode. Type: " + toString(uint32(curGenericField->m_type)) +
                        ", Is array: " + toString(curGenericField->m_isVectorType));
          }
        }
        else {
          switch (curGenericField->m_type)
          {
            case SERIALIZABLE_FIELD_TYPE::kReflectablePtr:
            {
              RTTIReflectablePtrFieldBase*
                curField = static_cast<RTTIReflectablePtrFieldBase*>(curGenericField);
              SPtr<IReflectable> childObject;

              if (!shallow) {
                childObject = curField->getValue(object);
              }

              uint32 objId = registerObjectPtr(childObject);
              COPY_TO_BUFFER(&objId, sizeof(uint32));
              break;
            }
            case SERIALIZABLE_FIELD_TYPE::kReflectable:
            {
              RTTIReflectableFieldBase*
                curField = static_cast<RTTIReflectableFieldBase*>(curGenericField);
              IReflectable& childObject = curField->getValue(object);

              buffer = complexTypeToBuffer(&childObject,
                                           buffer,
                                           bufferLength,
                                           bytesWritten,
                                           flushBufferCallback,
                                           shallow);
              if (nullptr == buffer) {
                si->onSerializationEnded(object, m_params);
                return nullptr;
              }
              break;
            }
            case SERIALIZABLE_FIELD_TYPE::kPlain:
            {
              RTTIPlainFieldBase* curField = static_cast<RTTIPlainFieldBase*>(curGenericField);

              uint32 typeSize = 0;
              if (curField->hasDynamicSize()) {
                typeSize = curField->getDynamicSize(object);
              }
              else {
                typeSize = curField->getTypeSize();
              }

              if ((*bytesWritten + typeSize) > bufferLength) {
                uint8* tempBuffer = 
                  reinterpret_cast<uint8*>(ge_stack_alloc(static_cast<SIZE_T>(typeSize)));
                curField->toBuffer(object, tempBuffer);

                buffer = dataBlockToBuffer(tempBuffer,
                                           typeSize,
                                           buffer,
                                           bufferLength,
                                           bytesWritten,
                                           flushBufferCallback);
                ge_stack_free(tempBuffer);

                if (nullptr == buffer || 0 == bufferLength) {
                  si->onSerializationEnded(object, m_params);
                  return nullptr;
                }
              }
              else {
                curField->toBuffer(object, buffer);
                buffer += typeSize;
                *bytesWritten += typeSize;
              }
              break;
            }
            case SERIALIZABLE_FIELD_TYPE::kDataBlock:
            {
              RTTIManagedDataBlockFieldBase*
                curField = static_cast<RTTIManagedDataBlockFieldBase*>(curGenericField);

              uint32 dataBlockSize = 0;
              SPtr<DataStream> blockStream = curField->getValue(object, dataBlockSize);

              //Data block size
              COPY_TO_BUFFER(&dataBlockSize, sizeof(uint32));

              //Data block data
              uint8* dataToStore = reinterpret_cast<uint8*>(ge_stack_alloc(dataBlockSize));
              blockStream->read(dataToStore, dataBlockSize);

              buffer = dataBlockToBuffer(dataToStore,
                                         dataBlockSize,
                                         buffer,
                                         bufferLength,
                                         bytesWritten,
                                         flushBufferCallback);
              ge_stack_free(dataToStore);

              if (nullptr == buffer || 0 == bufferLength) {
                si->onSerializationEnded(object, m_params);
                return nullptr;
              }
              break;
            }
            default:
              GE_EXCEPT(InternalErrorException,
                        "Error encoding data. Encountered a type I don't know "
                        "how to encode. Type: " +
                        toString(uint32(curGenericField->m_type)) +
                        ", Is array: " +
                        toString(curGenericField->m_isVectorType));
          }
        }
      }

      si->onSerializationEnded(object, m_params);

      si = si->getBaseClass();
      isBaseClass = true;

    } while (nullptr != si);  //Repeat until we reach the top of the inheritance hierarchy

    return buffer;
  }

  bool
  BinarySerializer::decodeEntry(const SPtr<DataStream>& data,
                                uint32 dataLength,
                                uint32& bytesRead,
                                SPtr<SerializedObject>& output,
                                bool copyData,
                                bool streamDataBlock) {
    ObjectMetaData objectMetaData;
    objectMetaData.objectMeta = 0;
    objectMetaData.typeId = 0;

    if (data->read(&objectMetaData, sizeof(ObjectMetaData)) != sizeof(ObjectMetaData)) {
      GE_EXCEPT(InternalErrorException, "Error decoding data.");
    }

    bytesRead += sizeof(ObjectMetaData);

    uint32 objectId = 0;
    uint32 objectTypeId = 0;
    bool objectIsBaseClass = false;
    decodeObjectMetaData(objectMetaData, objectId, objectTypeId, objectIsBaseClass);

    if (objectIsBaseClass) {
      GE_EXCEPT(InternalErrorException,
                "Encountered a base-class object while looking for a new object. "
                "Base class objects are only supposed to be parts of a larger object.");
    }

    RTTITypeBase* rtti = IReflectable::_getRTTIfromTypeId(objectTypeId);
    SerializedSubObject* serializedSubObject = nullptr;

    if (nullptr != rtti) {
      if (objectId > 0) {
        auto iterFind = m_interimObjectMap.find(objectId);
        if (m_interimObjectMap.end() == iterFind) {
          output = ge_shared_ptr_new<SerializedObject>();
          m_interimObjectMap.insert(make_pair(objectId, output));
        }
        else {
          output = iterFind->second;
        }
      }
      else { //Not a reflectable ptr referenced object
        output = ge_shared_ptr_new<SerializedObject>();
      }

      output->subObjects.emplace_back();
      serializedSubObject = &output->subObjects.back();
      serializedSubObject->typeId = objectTypeId;
    }

    while (bytesRead < dataLength) {
      int32 metaData = -1;
      if (data->read(&metaData, META_SIZE) != META_SIZE) {
        GE_EXCEPT(InternalErrorException, "Error decoding data.");
      }

      if (isObjectMetaData(metaData)) {
        //We've reached a new object or a base class of the current one
        ObjectMetaData objMetaData;
        objMetaData.objectMeta = 0;
        objMetaData.typeId = 0;

        data->seek(data->tell() - META_SIZE);
        if (data->read(&objMetaData, sizeof(ObjectMetaData)) != sizeof(ObjectMetaData)) {
          GE_EXCEPT(InternalErrorException, "Error decoding data.");
        }

        uint32 objId = 0;
        uint32 objTypeId = 0;
        bool objIsBaseClass = false;
        decodeObjectMetaData(objMetaData, objId, objTypeId, objIsBaseClass);

        //If it's a base class, get base class RTTI and handle that
        if (objIsBaseClass) {
          if (nullptr != rtti) {
            rtti = rtti->getBaseClass();
          }

          //Saved and current base classes don't match, so just skip over all that data
          if (nullptr == rtti || rtti->getRTTIId() != objTypeId) {
            rtti = nullptr;
          }

          if (nullptr != rtti) {
            output->subObjects.emplace_back();
            serializedSubObject = &output->subObjects.back();
            serializedSubObject->typeId = objTypeId;
          }

          bytesRead += sizeof(ObjectMetaData);
          continue;
        }
        else {
          //Found new object, we're done
          data->seek(data->tell() - sizeof(ObjectMetaData));
          return true;
        }
      }

      bytesRead += META_SIZE;

      bool isArray;
      SERIALIZABLE_FIELD_TYPE::E fieldType;
      uint16 fieldId;
      uint8 fieldSize;
      bool hasDynamicSize;
      bool terminator;
      decodeFieldMetaData(metaData,
                          fieldId,
                          fieldSize,
                          isArray,
                          fieldType,
                          hasDynamicSize,
                          terminator);

      if (terminator) {
        //We've processed the last field in this object, so return. Although we
        //return false we don't actually know if there is an object following
        //this one. However it doesn't matter since terminator fields are only
        //used for embedded objects that are all processed within this method
        //so we can compensate.
        return false;
      }

      RTTIField* curGenericField = nullptr;

      if (nullptr != rtti) {
        curGenericField = rtti->findField(fieldId);
      }

      if (nullptr != curGenericField) {
        if (!hasDynamicSize && curGenericField->getTypeSize() != fieldSize) {
          GE_EXCEPT(InternalErrorException,
                    "Data type mismatch. Type size stored in file and actual "
                    "type size don't match. (" +
                    toString(curGenericField->getTypeSize()) + " vs. " +
                    toString(fieldSize) + ")");
        }

        if (curGenericField->m_isVectorType != isArray) {
          GE_EXCEPT(InternalErrorException,
                    "Data type mismatch. One is array, other is a single type.");
        }

        if (curGenericField->m_type != fieldType) {
          GE_EXCEPT(InternalErrorException,
                    "Data type mismatch. Field types don't match. " +
                    toString(uint32(curGenericField->m_type)) + " vs. " +
                    toString(uint32(fieldType)));
        }
      }

      SPtr<SerializedInstance> serializedEntry;
      bool hasModification = false;

      uint32 arrayNumElems = 1;
      if (isArray) {
        if (data->read(&arrayNumElems, NUM_ELEM_FIELD_SIZE) != NUM_ELEM_FIELD_SIZE) {
          GE_EXCEPT(InternalErrorException, "Error decoding data.");
        }

        bytesRead += NUM_ELEM_FIELD_SIZE;

        SPtr<SerializedArray> serializedArray;
        if (nullptr != curGenericField) {
          serializedArray = ge_shared_ptr_new<SerializedArray>();
          serializedArray->numElements = arrayNumElems;
          serializedEntry = serializedArray;
          hasModification = true;
        }

        switch (fieldType)
        {
          case SERIALIZABLE_FIELD_TYPE::kReflectablePtr:
          {
            RTTIReflectablePtrFieldBase*
              curField = static_cast<RTTIReflectablePtrFieldBase*>(curGenericField);

            for (uint32 i = 0; i < arrayNumElems; ++i) {
              uint32 childObjectId = 0;
              SIZE_T cOIDSize = data->read(&childObjectId, COMPLEX_TYPE_FIELD_SIZE);
              if (COMPLEX_TYPE_FIELD_SIZE != cOIDSize) {
                GE_EXCEPT(InternalErrorException, "Error decoding data.");
              }

              bytesRead += COMPLEX_TYPE_FIELD_SIZE;

              if (nullptr != curField) {
                SPtr<SerializedObject> serializedArrayEntry = nullptr;

                if (childObjectId > 0) {
                  auto findObj = m_interimObjectMap.find(childObjectId);
                  if (m_interimObjectMap.end() == findObj) {
                    serializedArrayEntry = ge_shared_ptr_new<SerializedObject>();
                    m_interimObjectMap.insert(make_pair(childObjectId, serializedArrayEntry));
                  }
                  else {
                    serializedArrayEntry = findObj->second;
                  }
                }

                SerializedArrayEntry arrayEntry;
                arrayEntry.serialized = serializedArrayEntry;
                arrayEntry.index = i;
                serializedArray->entries[i] = arrayEntry;
              }
            }

            break;
          }
          case SERIALIZABLE_FIELD_TYPE::kReflectable:
          {
            RTTIReflectableFieldBase*
              curField = static_cast<RTTIReflectableFieldBase*>(curGenericField);

            for (uint32 i = 0; i < arrayNumElems; ++i) {
              SPtr<SerializedObject> serializedArrayEntry;
              decodeEntry(data,
                          dataLength,
                          bytesRead,
                          serializedArrayEntry,
                          copyData,
                          streamDataBlock);

              if (nullptr != curField) {
                SerializedArrayEntry arrayEntry;
                arrayEntry.serialized = serializedArrayEntry;
                arrayEntry.index = i;
                serializedArray->entries[i] = arrayEntry;
              }
            }
            break;
          }
          case SERIALIZABLE_FIELD_TYPE::kPlain:
          {
            RTTIPlainFieldBase*
              curField = static_cast<RTTIPlainFieldBase*>(curGenericField);

            for (uint32 i = 0; i < arrayNumElems; ++i) {
              uint32 typeSize = fieldSize;
              if (hasDynamicSize) {
                data->read(&typeSize, sizeof(uint32));
                data->seek(data->tell() - sizeof(uint32));
              }

              if (nullptr != curField) {
                SPtr<SerializedField> serializedField = ge_shared_ptr_new<SerializedField>();

                if (copyData) {
                  serializedField->value = 
                    reinterpret_cast<uint8*>(ge_alloc(static_cast<SIZE_T>(typeSize)));
                  data->read(serializedField->value, typeSize);

                  serializedField->ownsMemory = true;
                }
                else {
                  //Guaranteed not to be a file stream, as we check earlier
                  SPtr<MemoryDataStream>
                    memStream = static_pointer_cast<MemoryDataStream>(data);
                  serializedField->value = memStream->getCurrentPtr();
                  data->skip(typeSize);
                }

                serializedField->size = typeSize;
                SerializedArrayEntry arrayEntry;
                arrayEntry.serialized = serializedField;
                arrayEntry.index = i;
                serializedArray->entries[i] = arrayEntry;
              }
              else {
                data->skip(typeSize);
              }

              bytesRead += typeSize;
            }
            break;
          }
          default:
            GE_EXCEPT(InternalErrorException,
                      "Error decoding data. Encountered a type I don't know "
                      "how to decode. Type: " + toString(uint32(fieldType)) +
                      ", Is array: " + toString(isArray));
        }
      }
      else {
        switch (fieldType)
        {
          case SERIALIZABLE_FIELD_TYPE::kReflectablePtr:
          {
            RTTIReflectablePtrFieldBase*
              curField = static_cast<RTTIReflectablePtrFieldBase*>(curGenericField);

            uint32 childObjectId = 0;
            SIZE_T cOIDSize= data->read(&childObjectId, COMPLEX_TYPE_FIELD_SIZE);
            if (COMPLEX_TYPE_FIELD_SIZE != cOIDSize) {
              GE_EXCEPT(InternalErrorException, "Error decoding data.");
            }

            bytesRead += COMPLEX_TYPE_FIELD_SIZE;

            if (nullptr != curField) {
              SPtr<SerializedObject> serializedField = nullptr;

              if (childObjectId > 0) {
                auto findObj = m_interimObjectMap.find(childObjectId);
                if (m_interimObjectMap.end() == findObj) {
                  serializedField = ge_shared_ptr_new<SerializedObject>();
                  m_interimObjectMap.insert(make_pair(childObjectId, serializedField));
                }
                else {
                  serializedField = findObj->second;
                }
              }

              serializedEntry = serializedField;
              hasModification = true;
            }

            break;
          }
          case SERIALIZABLE_FIELD_TYPE::kReflectable:
          {
            RTTIReflectableFieldBase*
              curField = static_cast<RTTIReflectableFieldBase*>(curGenericField);
            SPtr<SerializedObject> serializedChildObj;
            decodeEntry(data,
                        dataLength,
                        bytesRead,
                        serializedChildObj,
                        copyData,
                        streamDataBlock);

            if (nullptr != curField) {
              serializedEntry = serializedChildObj;
              hasModification = true;
            }
            break;
          }
          case SERIALIZABLE_FIELD_TYPE::kPlain:
          {
            RTTIPlainFieldBase* curField = static_cast<RTTIPlainFieldBase*>(curGenericField);

            uint32 typeSize = fieldSize;
            if (hasDynamicSize) {
              data->read(&typeSize, sizeof(uint32));
              data->seek(data->tell() - sizeof(uint32));
            }

            if (nullptr != curField) {
              SPtr<SerializedField> serializedField = ge_shared_ptr_new<SerializedField>();
              if (copyData) {
                serializedField->value =
                  reinterpret_cast<uint8*>(ge_alloc(static_cast<SIZE_T>(typeSize)));
                data->read(serializedField->value, typeSize);
                serializedField->ownsMemory = true;
              }
              else {
                //Guaranteed not to be a file stream, as we check earlier
                SPtr<MemoryDataStream>
                  memStream = static_pointer_cast<MemoryDataStream>(data);
                serializedField->value = memStream->getCurrentPtr();
                data->skip(typeSize);
              }

              serializedField->size = typeSize;
              serializedEntry = serializedField;
              hasModification = true;
            }
            else {
              data->skip(typeSize);
            }

            bytesRead += typeSize;
            break;
          }
          case SERIALIZABLE_FIELD_TYPE::kDataBlock:
          {
            RTTIManagedDataBlockFieldBase*
              curField = static_cast<RTTIManagedDataBlockFieldBase*>(curGenericField);

            //Data block size
            uint32 dataBlockSize = 0;
            SIZE_T dBSize = data->read(&dataBlockSize, DATA_BLOCK_TYPE_FIELD_SIZE);
            if (DATA_BLOCK_TYPE_FIELD_SIZE != dBSize) {
              GE_EXCEPT(InternalErrorException, "Error decoding data.");
            }

            bytesRead += DATA_BLOCK_TYPE_FIELD_SIZE;

            //Data block data
            if (nullptr != curField) {
              SPtr<SerializedDataBlock>
                serializedDataBlock = ge_shared_ptr_new<SerializedDataBlock>();

              if (streamDataBlock || !copyData) {
                serializedDataBlock->stream = data;
                serializedDataBlock->offset = static_cast<uint32>(data->tell());
                data->skip(dataBlockSize);
              }
              else {
                uint8* dataBlockBuffer =
                  reinterpret_cast<uint8*>(ge_alloc(static_cast<SIZE_T>(dataBlockSize)));
                data->read(dataBlockBuffer, dataBlockSize);

                SPtr<DataStream> stream =
                  ge_shared_ptr_new<MemoryDataStream>(dataBlockBuffer, dataBlockSize);
                serializedDataBlock->stream = stream;
                serializedDataBlock->offset = 0;
              }

              serializedDataBlock->size = dataBlockSize;
              serializedEntry = serializedDataBlock;
              hasModification = true;
            }
            else {
              data->skip(dataBlockSize);
            }

            bytesRead += dataBlockSize;
            break;
          }
          default:
            GE_EXCEPT(InternalErrorException,
                      "Error decoding data. Encountered a type I don't know "
                      "how to decode. Type: " + toString(uint32(fieldType)) +
                      ", Is array: " + toString(isArray));
        }
      }

      if (hasModification) {
        SerializedEntry entry;
        entry.fieldId = curGenericField->m_uniqueId;
        entry.serialized = serializedEntry;
        serializedSubObject->entries.insert(make_pair(curGenericField->m_uniqueId, entry));
      }
    }

    return false;
  }

  void
  BinarySerializer::decodeEntry(const SPtr<IReflectable>& object,
                                const SPtr<SerializedObject>& serializableObject) {
    uint32 numSubObjects = static_cast<uint32>(serializableObject->subObjects.size());
    if (0 == numSubObjects) {
      return;
    }

    Vector<RTTITypeBase*> rttiTypes;
    for (int32 subObjectIdx = numSubObjects - 1; subObjectIdx >= 0; --subObjectIdx) {
      const SerializedSubObject& subObject = serializableObject->subObjects[subObjectIdx];

      RTTITypeBase* rtti = IReflectable::_getRTTIfromTypeId(subObject.typeId);
      if (rtti == nullptr) {
        continue;
      }

      rtti->onDeserializationStarted(object.get(), m_params);
      rttiTypes.push_back(rtti);

      uint32 numFields = rtti->getNumFields();
      for (uint32 fieldIdx = 0; fieldIdx < numFields; ++fieldIdx) {
        RTTIField* curGenericField = rtti->getField(fieldIdx);

        auto iterFindFieldData = subObject.entries.find(curGenericField->m_uniqueId);
        if (subObject.entries.end() == iterFindFieldData) {
          continue;
        }

        SPtr<SerializedInstance> entryData = iterFindFieldData->second.serialized;
        if (curGenericField->isArray()) {
          SPtr<SerializedArray> arrayData = static_pointer_cast<SerializedArray>(entryData);

          uint32 arrayNumElems = static_cast<uint32>(arrayData->numElements);
          curGenericField->setArraySize(object.get(), arrayNumElems);

          switch (curGenericField->m_type)
          {
            case SERIALIZABLE_FIELD_TYPE::kReflectablePtr:
            {
              RTTIReflectablePtrFieldBase*
                curField = static_cast<RTTIReflectablePtrFieldBase*>(curGenericField);

              for (auto& arrayElem : arrayData->entries) {
                SPtr<SerializedObject> arrayElemData =
                  static_pointer_cast<SerializedObject>(arrayElem.second.serialized);
                RTTITypeBase* childRtti = nullptr;

                if (nullptr != arrayElemData) {
                  childRtti = IReflectable::
                              _getRTTIfromTypeId(arrayElemData->getRootTypeId());
                }

                if (nullptr != childRtti) {
                  auto findObj = m_objectMap.find(arrayElemData);
                  if (m_objectMap.end() == findObj) {
                    SPtr<IReflectable> newObject = childRtti->newRTTIObject();
                    findObj = m_objectMap.insert(make_pair(arrayElemData,
                                                 ObjectToDecode(newObject,
                                                                arrayElemData))).first;
                  }

                  ObjectToDecode& objToDecode = findObj->second;

                  bool needsDecoding = (curField->getFlags() & RTTI_FIELD_FLAG::kWeakRef) == 0
                                       && !objToDecode.isDecoded;
                  if (needsDecoding) {
                    if (objToDecode.decodeInProgress) {
                      LOGWRN("Detected a circular reference when decoding. "
                             "Referenced object fields will be resolved in an "
                             "undefined order (i.e. one of the objects will "
                             "not be fully deserialized when assigned to its "
                             "field). Use RTTI_FIELD_FLAG::kWeakRef to get "
                             "rid of this warning and tell the system which "
                             "of the objects is allowed to be deserialized "
                             "after it is assigned to its field.");
                    }
                    else {
                      objToDecode.decodeInProgress = true;
                      decodeEntry(objToDecode.object, objToDecode.serializedObject);
                      objToDecode.decodeInProgress = false;
                      objToDecode.isDecoded = true;
                    }
                  }
                  curField->setArrayValue(object.get(), arrayElem.first, objToDecode.object);
                }
                else {
                  curField->setArrayValue(object.get(), arrayElem.first, nullptr);
                }
              }
            }
            break;
            case SERIALIZABLE_FIELD_TYPE::kReflectable:
            {
              RTTIReflectableFieldBase*
                curField = static_cast<RTTIReflectableFieldBase*>(curGenericField);

              for (auto& arrayElem : arrayData->entries) {
                SPtr<SerializedObject> arrayElemData =
                  static_pointer_cast<SerializedObject>(arrayElem.second.serialized);
                RTTITypeBase* childRtti = nullptr;

                if (nullptr != arrayElemData) {
                  childRtti = IReflectable::
                              _getRTTIfromTypeId(arrayElemData->getRootTypeId());
                }

                if (nullptr != childRtti) {
                  SPtr<IReflectable> newObject = childRtti->newRTTIObject();
                  decodeEntry(newObject, arrayElemData);
                  curField->setArrayValue(object.get(), arrayElem.first, *newObject);
                }
              }
              break;
            }
            case SERIALIZABLE_FIELD_TYPE::kPlain:
            {
              RTTIPlainFieldBase*
                curField = static_cast<RTTIPlainFieldBase*>(curGenericField);

              for (auto& arrayElem : arrayData->entries) {
                SPtr<SerializedField> fieldData =
                  static_pointer_cast<SerializedField>(arrayElem.second.serialized);
                if (nullptr != fieldData) {
                  curField->arrayElemFromBuffer(object.get(),
                                                arrayElem.first,
                                                fieldData->value);
                }
              }
            }
              break;
            default:
              break;
          }
        }
        else {
          switch (curGenericField->m_type)
          {
            case SERIALIZABLE_FIELD_TYPE::kReflectablePtr:
            {
              RTTIReflectablePtrFieldBase*
                curField = static_cast<RTTIReflectablePtrFieldBase*>(curGenericField);

              SPtr<SerializedObject> fieldObjectData =
                static_pointer_cast<SerializedObject>(entryData);
              RTTITypeBase* childRtti = nullptr;

              if (fieldObjectData != nullptr)
                childRtti = IReflectable::
                            _getRTTIfromTypeId(fieldObjectData->getRootTypeId());

              if (nullptr != childRtti) {
                auto findObj = m_objectMap.find(fieldObjectData);
                if (m_objectMap.end() == findObj) {
                  SPtr<IReflectable> newObject = childRtti->newRTTIObject();
                  findObj = m_objectMap.insert(make_pair(fieldObjectData,
                                               ObjectToDecode(newObject,
                                                              fieldObjectData))).first;
                }

                ObjectToDecode& objToDecode = findObj->second;

                bool needsDecoding = (curField->getFlags() & RTTI_FIELD_FLAG::kWeakRef) == 0
                                      && !objToDecode.isDecoded;
                if (needsDecoding) {
                  if (objToDecode.decodeInProgress) {
                    LOGWRN("Detected a circular reference when decoding. "
                           "Referenced object's fields will be resolved in an "
                           "undefined order (i.e. one of the objects will not "
                           "be fully deserialized when assigned to its field)."
                           " Use RTTI_Flag_WeakRef to get rid of this warning "
                           "and tell the system which of the objects is "
                           "allowed to be deserialized after it is assigned "
                           "to its field.");
                  }
                  else {
                    objToDecode.decodeInProgress = true;
                    decodeEntry(objToDecode.object, objToDecode.serializedObject);
                    objToDecode.decodeInProgress = false;
                    objToDecode.isDecoded = true;
                  }
                }

                curField->setValue(object.get(), objToDecode.object);
              }
              else {
                curField->setValue(object.get(), nullptr);
              }
            }
            break;
            case SERIALIZABLE_FIELD_TYPE::kReflectable:
            {
              RTTIReflectableFieldBase*
                curField = static_cast<RTTIReflectableFieldBase*>(curGenericField);

              SPtr<SerializedObject> fieldObjectData =
                static_pointer_cast<SerializedObject>(entryData);
              RTTITypeBase* childRtti = nullptr;

              if (nullptr != fieldObjectData) {
                childRtti = IReflectable::
                            _getRTTIfromTypeId(fieldObjectData->getRootTypeId());
              }

              if (nullptr != childRtti) {
                SPtr<IReflectable> newObject = childRtti->newRTTIObject();
                decodeEntry(newObject, fieldObjectData);
                curField->setValue(object.get(), *newObject);
              }
              break;
            }
            case SERIALIZABLE_FIELD_TYPE::kPlain:
            {
              RTTIPlainFieldBase*
                curField = static_cast<RTTIPlainFieldBase*>(curGenericField);

              SPtr<SerializedField> fieldData =
                static_pointer_cast<SerializedField>(entryData);
              if (nullptr != fieldData) {
                curField->fromBuffer(object.get(), fieldData->value);
              }
            }
            break;
            case SERIALIZABLE_FIELD_TYPE::kDataBlock:
            {
              RTTIManagedDataBlockFieldBase*
                curField = static_cast<RTTIManagedDataBlockFieldBase*>(curGenericField);

              SPtr<SerializedDataBlock> fieldData =
                static_pointer_cast<SerializedDataBlock>(entryData);
              if (nullptr != fieldData) {
                fieldData->stream->seek(fieldData->offset);
                curField->setValue(object.get(), fieldData->stream, fieldData->size);
              }
              break;
            }
          }
        }
      }
    }

    for (auto iterFind = rttiTypes.begin(); iterFind != rttiTypes.end(); ++iterFind) {
      (*iterFind)->onDeserializationEnded(object.get(), m_params);
    }
  }

  uint32
  BinarySerializer::encodeFieldMetaData(uint16 id,
                                        uint8 size,
                                        bool array,
                                        SERIALIZABLE_FIELD_TYPE::E type,
                                        bool hasDynamicSize,
                                        bool terminator) {
    // If O == 0 - Meta contains field information (Encoded using this method)
    //// Encoding: IIII IIII IIII IIII SSSS SSSS xTYP DCAO
    //// I - Id
    //// S - Size
    //// C - Complex
    //// A - Array
    //// D - Data block
    //// P - Complex ptr
    //// O - Object descriptor
    //// Y - Plain field has dynamic size
    //// T - Terminator (last field in an object)

    return (id << 16 | size << 8 |
           (array ? 0x02 : 0) |
           ((type == SERIALIZABLE_FIELD_TYPE::kDataBlock) ? 0x04 : 0) |
           ((type == SERIALIZABLE_FIELD_TYPE::kReflectable) ? 0x08 : 0) |
           ((type == SERIALIZABLE_FIELD_TYPE::kReflectablePtr) ? 0x10 : 0) |
           (hasDynamicSize ? 0x20 : 0) |
           (terminator ? 0x40 : 0));
    //TODO: Low priority. Technically I could encode this much more tightly,
    //and use var-ints for ID
  }

  void
  BinarySerializer::decodeFieldMetaData(uint32 encodedData,
                                        uint16& id,
                                        uint8& size,
                                        bool& array,
                                        SERIALIZABLE_FIELD_TYPE::E& type,
                                        bool& hasDynamicSize,
                                        bool& terminator) {
    if (isObjectMetaData(encodedData)) {
      GE_EXCEPT(InternalErrorException,
                "Meta data represents an object description but is trying to "
                "be decoded as a field descriptor.");
    }

    terminator = (encodedData & 0x40) != 0;
    hasDynamicSize = (encodedData & 0x20) != 0;

    if ((encodedData & 0x10) != 0) {
      type = SERIALIZABLE_FIELD_TYPE::kReflectablePtr;
    }
    else if ((encodedData & 0x08) != 0) {
      type = SERIALIZABLE_FIELD_TYPE::kReflectable;
    }
    else if ((encodedData & 0x04) != 0) {
      type = SERIALIZABLE_FIELD_TYPE::kDataBlock;
    }
    else {
      type = SERIALIZABLE_FIELD_TYPE::kPlain;
    }

    array = (encodedData & 0x02) != 0;
    size = static_cast<uint8>((encodedData >> 8) & 0xFF);
    id = static_cast<uint16>((encodedData >> 16) & 0xFFFF);
  }

  BinarySerializer::ObjectMetaData
  BinarySerializer::encodeObjectMetaData(uint32 objId, uint32 objTypeId, bool isBaseClass) {
    // If O == 1 - Meta contains object instance information (Encoded by encodeObjectMetaData)
    //// Encoding: SSSS SSSS SSSS SSSS xxxx xxxx xxxx xxBO
    //// S - Size of the object identifier
    //// O - Object descriptor
    //// B - Base class indicator

    if (objId > 1073741823) {
      GE_EXCEPT(InvalidParametersException,
                "Object ID is larger than we can store (max 30 bits): " +
                toString(objId));
    }

    ObjectMetaData metaData;
    metaData.objectMeta = (objId << 2) | (isBaseClass ? 0x02 : 0) | 0x01;
    metaData.typeId = objTypeId;
    return metaData;
  }

  void
  BinarySerializer::decodeObjectMetaData(BinarySerializer::ObjectMetaData encodedData,
                                         uint32& objId,
                                         uint32& objTypeId,
                                         bool& isBaseClass) {
    if (!isObjectMetaData(encodedData.objectMeta)) {
      GE_EXCEPT(InternalErrorException,
                "Meta data represents a field description but is trying to be "
                "decoded as an object descriptor.");
    }

    objId = (encodedData.objectMeta >> 2) & 0x3FFFFFFF;
    isBaseClass = (encodedData.objectMeta & 0x02) != 0;
    objTypeId = encodedData.typeId;
  }

  bool
  BinarySerializer::isObjectMetaData(uint32 encodedData) {
    return ((encodedData & 0x01) != 0);
  }

  uint8*
  BinarySerializer::complexTypeToBuffer(IReflectable* object,
                                uint8* buffer,
                                uint32& bufferLength,
                                uint32* bytesWritten,
                                function<uint8*(uint8*, uint32, uint32&)> flushBufferCallback,
                                bool shallow) {
    if (nullptr != object) {
      buffer = encodeEntry(object,
                           0,
                           buffer,
                           bufferLength,
                           bytesWritten,
                           flushBufferCallback,
                           shallow);

      //Encode terminator field
      //Complex types require terminator fields because they can be embedded
      //within other complex types and we need to know when their fields end
      //and parent's resume
      uint32 metaData = encodeFieldMetaData(0,
                                            0,
                                            false,
                                            SERIALIZABLE_FIELD_TYPE::kPlain,
                                            false,
                                            true);
      COPY_TO_BUFFER(&metaData, META_SIZE);
    }

    return buffer;
  }

  uint8*
  BinarySerializer::dataBlockToBuffer(uint8* data,
                              uint32 size,
                              uint8* buffer,
                              uint32& bufferLength,
                              uint32* bytesWritten,
                              function<uint8*(uint8*, uint32, uint32&)> flushBufferCallback) {
    uint32 remainingSize = size;
    while (remainingSize > 0) {
      uint32 remainingSpaceInBuffer = bufferLength - *bytesWritten;

      if (remainingSize <= remainingSpaceInBuffer) {
        COPY_TO_BUFFER(data, remainingSize);
        remainingSize = 0;
      }
      else {
        memcpy(buffer, data, remainingSpaceInBuffer);
        buffer += remainingSpaceInBuffer;
        *bytesWritten += remainingSpaceInBuffer;
        data += remainingSpaceInBuffer;
        remainingSize -= remainingSpaceInBuffer;

        m_totalBytesWritten += *bytesWritten;
        buffer = flushBufferCallback(buffer - *bytesWritten, *bytesWritten, bufferLength);
        if (nullptr == buffer || 0 == bufferLength) {
          return nullptr;
        }
        *bytesWritten = 0;
      }
    }

    return buffer;
  }

  uint32
  BinarySerializer::findOrCreatePersistentId(IReflectable* object) {
    void* ptrAddress = reinterpret_cast<void*>(object);

    auto findIter = m_objectAddrToId.find(ptrAddress);
    if (m_objectAddrToId.end() != findIter) {
      return findIter->second;
    }

    uint32 objId = m_lastUsedObjectId++;
    m_objectAddrToId.insert(make_pair(ptrAddress, objId));

    return objId;
  }

  uint32
  BinarySerializer::registerObjectPtr(SPtr<IReflectable> object) {
    if (nullptr == object) {
      return 0;
    }

    void* ptrAddress = reinterpret_cast<void*>(object.get());

    auto iterFind = m_objectAddrToId.find(ptrAddress);
    if (m_objectAddrToId.end() == iterFind) {
      uint32 objId = findOrCreatePersistentId(object.get());

      m_objectsToEncode.push_back(ObjectToEncode(objId, object));
      m_objectAddrToId.insert(make_pair(ptrAddress, objId));

      return objId;
    }

    return iterFind->second;
  }
}

#undef COPY_TO_BUFFER
