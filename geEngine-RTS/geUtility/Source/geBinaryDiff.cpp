/*****************************************************************************/
/**
 * @file    geBinaryDiff.cpp
 * @author  Samuel Prince (samuel.prince.quezada@gmail.com)
 * @date    2017/11/03
 * @brief   Represents an interface RTTI objects need to implement if they want
 *          to provide custom "diff" generation and applying.
 *
 * Represents an interface RTTI objects need to implement if they want to
 * provide custom "diff" generation and applying.
 *
 * @bug     No known bugs.
 */
/*****************************************************************************/

/*****************************************************************************/
/**
 * Includes
 */
/*****************************************************************************/
#include "geBinaryDiff.h"
#include "geSerializedObject.h"
#include "geBinarySerializer.h"
#include "geBinaryCloner.h"
#include "geRTTIType.h"
#include "geDataStream.h"
#include "geMath.h"

namespace geEngineSDK {
  using std::static_pointer_cast;
  using std::make_pair;

  SPtr<SerializedObject>
  IDiff::generateDiff(const SPtr<SerializedObject>& orgObj,
                      const SPtr<SerializedObject>& newObj) {
    ObjectMap objectMap;
    return generateDiff(orgObj, newObj, objectMap);
  }

  SPtr<SerializedInstance>
  IDiff::generateDiff(RTTITypeBase* /*rtti*/,
                      uint32 fieldType,
                      const SPtr<SerializedInstance>& orgData,
                      const SPtr<SerializedInstance>& newData,
                      ObjectMap& objectMap) {
    SPtr<SerializedInstance> modification;
    switch (fieldType)
    {
      case SERIALIZABLE_FIELD_TYPE::kReflectablePtr:
      case SERIALIZABLE_FIELD_TYPE::kReflectable:
      {
        SPtr<SerializedObject>
          orgObjData = static_pointer_cast<SerializedObject>(orgData);
        SPtr<SerializedObject>
          newObjData = static_pointer_cast<SerializedObject>(newData);

        auto iterFind = objectMap.find(newObjData);
        if (iterFind != objectMap.end()) {
          modification = iterFind->second;
        }
        else {
          RTTITypeBase* childRtti = nullptr;
          if (orgObjData->getRootTypeId() == newObjData->getRootTypeId()) {
            childRtti = IReflectable::_getRTTIfromTypeId(newObjData->getRootTypeId());
          }

          SPtr<SerializedObject> objectDiff;
          if (nullptr != childRtti) {
            IDiff& handler = childRtti->getDiffHandler();
            objectDiff = handler.generateDiff(orgObjData, newObjData, objectMap);
          }

          if (nullptr != objectDiff) {
            objectMap[newObjData] = objectDiff;
          }

          modification = objectDiff;
        }
      }
        break;
      case SERIALIZABLE_FIELD_TYPE::kPlain:
      {
        SPtr<SerializedField> 
          orgFieldData = static_pointer_cast<SerializedField>(orgData);
        SPtr<SerializedField> 
          newFieldData = static_pointer_cast<SerializedField>(newData);

        bool isModified = orgFieldData->size != newFieldData->size;
        if (!isModified) {
          isModified = memcmp(orgFieldData->value,
                              newFieldData->value,
                              newFieldData->size) != 0;
        }

        if (isModified) {
          modification = newFieldData->clone();
        }
      }
        break;
      case SERIALIZABLE_FIELD_TYPE::kDataBlock:
      {
        SPtr<SerializedDataBlock>
          orgFieldData = static_pointer_cast<SerializedDataBlock>(orgData);
        SPtr<SerializedDataBlock>
          newFieldData = static_pointer_cast<SerializedDataBlock>(newData);

        bool isModified = orgFieldData->size != newFieldData->size;
        if (!isModified) {
          uint8* orgStreamData = nullptr;
          if (orgFieldData->stream->isFile()) {
            SIZE_T readSize = static_cast<SIZE_T>(orgFieldData->size);
            orgStreamData = reinterpret_cast<uint8*>(ge_stack_alloc(readSize));
            orgFieldData->stream->seek(static_cast<SIZE_T>(orgFieldData->offset));
            orgFieldData->stream->read(orgStreamData, readSize);
          }
          else {
            SPtr<MemoryDataStream>
              orgMemStream = static_pointer_cast<MemoryDataStream>(orgFieldData->stream);
            orgStreamData = orgMemStream->getCurrentPtr();
          }

          uint8* newStreamData = nullptr;
          if (newFieldData->stream->isFile()) {
            SIZE_T readSize = static_cast<SIZE_T>(newFieldData->size);
            newStreamData = reinterpret_cast<uint8*>(ge_stack_alloc(readSize));
            newFieldData->stream->seek(newFieldData->offset);
            newFieldData->stream->read(newStreamData, readSize);
          }
          else {
            SPtr<MemoryDataStream>
              newMemStream = static_pointer_cast<MemoryDataStream>(newFieldData->stream);
            newStreamData = newMemStream->getCurrentPtr();
          }

          isModified = memcmp(orgStreamData, newStreamData, newFieldData->size) != 0;

          if (newFieldData->stream->isFile()) {
            ge_stack_free(newStreamData);
          }

          if (orgFieldData->stream->isFile()) {
            ge_stack_free(orgStreamData);
          }
        }

        if (isModified) {
          modification = newFieldData->clone();
        }
      }
      break;
    }

    return modification;
  }

  void
  IDiff::applyDiff(const SPtr<IReflectable>& object, const SPtr<SerializedObject>& diff) {
    static const UnorderedMap<String, uint64> dummyParams;

    Vector<DiffCommand> commands;

    DiffObjectMap objectMap;
    applyDiff(object, diff, objectMap, commands);

    IReflectable* destObject = nullptr;
    Stack<IReflectable*> objectStack;
    Vector<RTTITypeBase*> rttiTypes;

    for (auto& command : commands) {
      bool isArray = (command.type & DIFF_COMMAND_TYPE::kArrayFlag) != 0;
      DIFF_COMMAND_TYPE::E type = static_cast<DIFF_COMMAND_TYPE::E>(command.type & 0xF);

      switch (type)
      {
        case DIFF_COMMAND_TYPE::kArraySize:
          command.field->setArraySize(destObject, command.arraySize);
          break;
        case DIFF_COMMAND_TYPE::kObjectStart:
        {
          destObject = command.object.get();
          objectStack.push(destObject);

          RTTITypeBase* curRtti = destObject->getRTTI();
          while (nullptr != curRtti) {
            rttiTypes.push_back(curRtti);
            curRtti = curRtti->getBaseClass();
          }

          //Call base class first, followed by derived classes
          for (auto iter = rttiTypes.rbegin(); iter != rttiTypes.rend(); ++iter) {
            (*iter)->onDeserializationStarted(destObject, dummyParams);
          }
        }
          break;
        case DIFF_COMMAND_TYPE::kObjectEnd:
        {
          Stack<RTTITypeBase*> sRTTITypes;
          RTTITypeBase* curRtti = destObject->getRTTI();
          while (nullptr != curRtti) {
            sRTTITypes.push(curRtti);
            curRtti = curRtti->getBaseClass();
          }

          while (!sRTTITypes.empty()) {
            sRTTITypes.top()->onDeserializationEnded(destObject, dummyParams);
            sRTTITypes.pop();
          }

          objectStack.pop();

          if (!objectStack.empty()) {
            destObject = objectStack.top();
          }
          else {
            destObject = nullptr;
          }
        }
          break;
        default:
          break;
        }

        if (isArray) {
          uint32 arrayInx = static_cast<uint32>(command.arrayIdx);

          switch (type)
          {
            case DIFF_COMMAND_TYPE::kReflectablePtr:
            {
              RTTIReflectablePtrFieldBase*
                field = static_cast<RTTIReflectablePtrFieldBase*>(command.field);
              field->setArrayValue(destObject, arrayInx, command.object);
            }
              break;
            case DIFF_COMMAND_TYPE::kReflectable:
            {
              RTTIReflectableFieldBase*
                field = static_cast<RTTIReflectableFieldBase*>(command.field);
              field->setArrayValue(destObject, arrayInx, *command.object);
            }
              break;
            case DIFF_COMMAND_TYPE::kPlain:
            {
              RTTIPlainFieldBase*
                field = static_cast<RTTIPlainFieldBase*>(command.field);
              field->arrayElemFromBuffer(destObject, arrayInx, command.value);
            }
              break;
            default:
              break;
          }
        }
        else {
          switch (type)
          {
            case DIFF_COMMAND_TYPE::kReflectablePtr:
            {
              RTTIReflectablePtrFieldBase*
                field = static_cast<RTTIReflectablePtrFieldBase*>(command.field);
              field->setValue(destObject, command.object);
            }
              break;
            case DIFF_COMMAND_TYPE::kReflectable:
            {
              RTTIReflectableFieldBase*
                field = static_cast<RTTIReflectableFieldBase*>(command.field);
              field->setValue(destObject, *command.object);
            }
              break;
            case DIFF_COMMAND_TYPE::kPlain:
            {
              RTTIPlainFieldBase*
                field = static_cast<RTTIPlainFieldBase*>(command.field);
              field->fromBuffer(destObject, command.value);
            }
              break;
            case DIFF_COMMAND_TYPE::kDataBlock:
            {
              RTTIManagedDataBlockFieldBase*
                field = static_cast<RTTIManagedDataBlockFieldBase*>(command.field);
              field->setValue(destObject, command.streamValue, command.size);
            }
              break;
            default:
              break;
        }
      }
    }
  }

  void
  IDiff::applyDiff(RTTITypeBase* rtti,
                   const SPtr<IReflectable>& object,
                   const SPtr<SerializedObject>& diff,
                   DiffObjectMap& objectMap,
                   Vector<DiffCommand>& diffCommands) {
    IDiff& diffHandler = rtti->getDiffHandler();
    diffHandler.applyDiff(object, diff, objectMap, diffCommands);
  }

  SPtr<SerializedObject>
  BinaryDiff::generateDiff(const SPtr<SerializedObject>& orgObj,
                           const SPtr<SerializedObject>& newObj,
                           ObjectMap& objectMap) {
    SPtr<SerializedObject> output;
    for (auto& subObject : newObj->subObjects) {
      RTTITypeBase* rtti = IReflectable::_getRTTIfromTypeId(subObject.typeId);
      if (nullptr == rtti) {
        continue;
      }

      SerializedSubObject* orgSubObject = nullptr;
      for (auto& curSubObject : orgObj->subObjects) {
        if (curSubObject.typeId == subObject.typeId) {
          orgSubObject = &curSubObject;
          break;
        }
      }

      SerializedSubObject* diffSubObject = nullptr;
      for (auto& newEntry : subObject.entries) {
        RTTIField* genericField = rtti->findField(newEntry.first);
        if (nullptr == genericField) {
          continue;
        }

        SPtr<SerializedInstance> newEntryData = newEntry.second.serialized;
        SPtr<SerializedInstance> orgEntryData;

        if (nullptr != orgSubObject) {
          auto orgEntryFind = orgSubObject->entries.find(newEntry.first);
          if (orgEntryFind != orgSubObject->entries.end()) {
            orgEntryData = orgEntryFind->second.serialized;
          }
        }

        SPtr<SerializedInstance> modification;
        bool hasModification = false;
        if (genericField->isArray()) {
          SPtr<SerializedArray>
            orgArrayData = static_pointer_cast<SerializedArray>(orgEntryData);
          SPtr<SerializedArray>
            newArrayData = static_pointer_cast<SerializedArray>(newEntryData);

          SPtr<SerializedArray> serializedArray;

          if (nullptr != newEntryData && nullptr != orgEntryData) {
            for (auto& arrayEntryPair : newArrayData->entries) {
              SPtr<SerializedInstance> arrayModification;

              auto iterFind = orgArrayData->entries.find(arrayEntryPair.first);
              if (orgArrayData->entries.end() == iterFind) {
                arrayModification = arrayEntryPair.second.serialized->clone();
              }
              else {
                arrayModification = IDiff::generateDiff(rtti,
                                                        genericField->m_type,
                                                        iterFind->second.serialized,
                                                        arrayEntryPair.second.serialized,
                                                        objectMap);
              }

              if (nullptr != arrayModification) {
                if (nullptr == serializedArray) {
                  serializedArray = ge_shared_ptr_new<SerializedArray>();
                  serializedArray->numElements = newArrayData->numElements;
                }

                SerializedArrayEntry arrayEntry;
                arrayEntry.index = arrayEntryPair.first;
                arrayEntry.serialized = arrayModification;
                serializedArray->entries[arrayEntryPair.first] = arrayEntry;
              }
            }
          }
          else if (nullptr == newEntryData) {
            serializedArray = ge_shared_ptr_new<SerializedArray>();
          }
          else if (nullptr == orgEntryData) {
            serializedArray = static_pointer_cast<SerializedArray>(newArrayData->clone());
          }

          modification = serializedArray;
          hasModification = nullptr != modification;
        }
        else {
          if (nullptr != newEntryData && nullptr != orgEntryData) {
            modification = IDiff::generateDiff(rtti,
                                               genericField->m_type,
                                               orgEntryData,
                                               newEntryData,
                                               objectMap);
            hasModification = nullptr != modification;
          }
          else if (nullptr == newEntryData) {
            switch (genericField->m_type)
            {
              case SERIALIZABLE_FIELD_TYPE::kPlain:
                modification = ge_shared_ptr_new<SerializedField>();
                break;
              case SERIALIZABLE_FIELD_TYPE::kDataBlock:
                modification = ge_shared_ptr_new<SerializedDataBlock>();
                break;
              default:
                break;
            }

            hasModification = true;
          }
          else if (nullptr == orgEntryData) {
            modification = newEntryData->clone();
            hasModification = nullptr != modification;
          }
        }

        if (hasModification) {
          if (nullptr == output) {
            output = ge_shared_ptr_new<SerializedObject>();
          }

          if (nullptr == diffSubObject) {
            output->subObjects.emplace_back();
            diffSubObject = &output->subObjects.back();
            diffSubObject->typeId = rtti->getRTTIId();
          }

          SerializedEntry modificationEntry;
          modificationEntry.fieldId = static_cast<uint32>(genericField->m_uniqueId);
          modificationEntry.serialized = modification;
          diffSubObject->entries[genericField->m_uniqueId] = modificationEntry;
        }
      }
    }

    return output;
  }

  void
  BinaryDiff::applyDiff(const SPtr<IReflectable>& object,
                        const SPtr<SerializedObject>& diff,
                        DiffObjectMap& objectMap,
                        Vector<DiffCommand>& diffCommands) {
    static const UnorderedMap<String, uint64> dummyParams;

    if (nullptr == object ||
        nullptr == diff ||
        object->getTypeId() != diff->getRootTypeId()) {
      return;
    }

    DiffCommand objStartCommand;
    objStartCommand.field = nullptr;
    objStartCommand.type = DIFF_COMMAND_TYPE::kObjectStart;
    objStartCommand.object = object;

    diffCommands.push_back(objStartCommand);

    Stack<RTTITypeBase*> rttiTypes;
    for (auto& subObject : diff->subObjects) {
      for (auto& diffEntry : subObject.entries) {
        RTTITypeBase* rtti = IReflectable::_getRTTIfromTypeId(subObject.typeId);
        if (nullptr == rtti) {
          continue;
        }

        if (!object->isDerivedFrom(rtti)) {
          continue;
        }

        rtti->onSerializationStarted(object.get(), dummyParams);
        rttiTypes.push(rtti);

        RTTIField* genericField = rtti->findField(diffEntry.first);
        if (nullptr == genericField) {
          continue;
        }

        SPtr<SerializedInstance> diffData = diffEntry.second.serialized;

        if (genericField->isArray()) {
          SPtr<SerializedArray> diffArray = static_pointer_cast<SerializedArray>(diffData);

          uint32 numArrayElements = diffArray->numElements;

          DiffCommand arraySizeCommand;
          arraySizeCommand.field = genericField;
          arraySizeCommand.type = DIFF_COMMAND_TYPE::kArraySize |
                                  DIFF_COMMAND_TYPE::kArrayFlag;
          arraySizeCommand.arraySize = static_cast<uint32>(numArrayElements);

          diffCommands.push_back(arraySizeCommand);

          switch (genericField->m_type)
          {
            case SERIALIZABLE_FIELD_TYPE::kReflectablePtr:
            {
              RTTIReflectablePtrFieldBase*
                field = static_cast<RTTIReflectablePtrFieldBase*>(genericField);

              uint32 orgArraySize = genericField->getArraySize(object.get());
              for (auto& arrayElem : diffArray->entries) {
                SPtr<SerializedObject> arrayElemData = 
                  static_pointer_cast<SerializedObject>(arrayElem.second.serialized);

                DiffCommand command;
                command.field = genericField;
                command.type = DIFF_COMMAND_TYPE::kReflectablePtr |
                               DIFF_COMMAND_TYPE::kArrayFlag;
                command.arrayIdx = arrayElem.first;

                if (nullptr == arrayElemData) {
                  command.object = nullptr;
                  diffCommands.push_back(command);
                }
                else {
                  bool needsNewObject = arrayElem.first >= orgArraySize;

                  if (!needsNewObject) {
                    SPtr<IReflectable>
                      childObj = field->getArrayValue(object.get(), arrayElem.first);
                    if (nullptr != childObj) {
                      IDiff::applyDiff(childObj->getRTTI(),
                                       childObj,
                                       arrayElemData,
                                       objectMap,
                                       diffCommands);
                      command.object = childObj;
                    }
                    else {
                      needsNewObject = true;
                    }
                  }

                  if (needsNewObject) {
                    RTTITypeBase* childRtti = 
                      IReflectable::_getRTTIfromTypeId(arrayElemData->getRootTypeId());
                    if (nullptr != childRtti) {
                      auto findObj = objectMap.find(arrayElemData);
                      if (objectMap.end() == findObj) {
                        SPtr<IReflectable> newObject = childRtti->newRTTIObject();
                        findObj = objectMap.insert(make_pair(arrayElemData, newObject)).first;
                      }

                      IDiff::applyDiff(childRtti,
                                       findObj->second,
                                       arrayElemData,
                                       objectMap,
                                       diffCommands);
                      command.object = findObj->second;
                      diffCommands.push_back(command);
                    }
                    else {
                      command.object = nullptr;
                      diffCommands.push_back(command);
                    }
                  }
                }
              }
            }
              break;
            case SERIALIZABLE_FIELD_TYPE::kReflectable:
            {
              RTTIReflectableFieldBase* 
                field = static_cast<RTTIReflectableFieldBase*>(genericField);

              uint32 orgArraySize = genericField->getArraySize(object.get());

              Vector<SPtr<IReflectable>> newArrayElements(numArrayElements);
              uint32 minArrayLength = Math::min(orgArraySize, numArrayElements);
              for (uint32 i = 0; i < minArrayLength; ++i) {
                IReflectable& childObj = field->getArrayValue(object.get(), i);
                newArrayElements[i] = BinaryCloner::clone(&childObj, true);
              }

              for (auto& arrayElem : diffArray->entries) {
                SPtr<SerializedObject> arrayElemData = 
                  static_pointer_cast<SerializedObject>(arrayElem.second.serialized);

                if (arrayElem.first < orgArraySize) {
                  SPtr<IReflectable> childObj = newArrayElements[arrayElem.first];
                  IDiff::applyDiff(childObj->getRTTI(),
                                   childObj,
                                   arrayElemData,
                                   objectMap,
                                   diffCommands);
                }
                else {
                  RTTITypeBase* childRtti = 
                    IReflectable::_getRTTIfromTypeId(arrayElemData->getRootTypeId());
                  if (nullptr != childRtti) {
                    SPtr<IReflectable> newObject = childRtti->newRTTIObject();
                    IDiff::applyDiff(childRtti,
                                     newObject,
                                     arrayElemData,
                                     objectMap,
                                     diffCommands);
                    newArrayElements[arrayElem.first] = newObject;
                  }
                }
              }

              for (uint32 i = 0; i < numArrayElements; ++i) {
                DiffCommand command;
                command.field = genericField;
                command.type = DIFF_COMMAND_TYPE::kReflectable |
                               DIFF_COMMAND_TYPE::kArrayFlag;
                command.arrayIdx = i;
                command.object = newArrayElements[i];

                diffCommands.push_back(command);
              }
            }
              break;
            case SERIALIZABLE_FIELD_TYPE::kPlain:
            {
              for (auto& arrayElem : diffArray->entries) {
                SPtr<SerializedField> fieldData =
                  static_pointer_cast<SerializedField>(arrayElem.second.serialized);
                if (nullptr != fieldData) {
                  DiffCommand command;
                  command.field = genericField;
                  command.type = DIFF_COMMAND_TYPE::kPlain |
                                 DIFF_COMMAND_TYPE::kArrayFlag;
                  command.value = fieldData->value;
                  command.size = fieldData->size;
                  command.arrayIdx = arrayElem.first;

                  diffCommands.push_back(command);
                }
              }
            }
              break;
            default:
              break;
          }
        }
        else {
          switch (genericField->m_type)
          {
            case SERIALIZABLE_FIELD_TYPE::kReflectablePtr:
            {
              RTTIReflectablePtrFieldBase*
                field = static_cast<RTTIReflectablePtrFieldBase*>(genericField);
              SPtr<SerializedObject> 
                fieldObjectData = static_pointer_cast<SerializedObject>(diffData);

              DiffCommand command;
              command.field = genericField;
              command.type = DIFF_COMMAND_TYPE::kReflectablePtr;

              if (nullptr == fieldObjectData) {
                command.object = nullptr;
              }
              else {
                SPtr<IReflectable> childObj = field->getValue(object.get());
                if (nullptr == childObj) {
                  RTTITypeBase* childRtti = 
                    IReflectable::_getRTTIfromTypeId(fieldObjectData->getRootTypeId());
                  if (nullptr != childRtti) {
                    auto findObj = objectMap.find(fieldObjectData);
                    if (objectMap.end() == findObj) {
                      SPtr<IReflectable> newObject = childRtti->newRTTIObject();
                      findObj = objectMap.insert(make_pair(fieldObjectData, newObject)).first;
                    }

                    IDiff::applyDiff(childRtti,
                                     findObj->second,
                                     fieldObjectData,
                                     objectMap,
                                     diffCommands);
                    command.object = findObj->second;
                  }
                  else {
                    command.object = nullptr;
                  }
                }
                else {
                  IDiff::applyDiff(childObj->getRTTI(),
                                   childObj,
                                   fieldObjectData,
                                   objectMap,
                                   diffCommands);
                  command.object = childObj;
                }
              }

              diffCommands.push_back(command);
            }
              break;
            case SERIALIZABLE_FIELD_TYPE::kReflectable:
            {
              RTTIReflectableFieldBase*
                field = static_cast<RTTIReflectableFieldBase*>(genericField);
              SPtr<SerializedObject>
                fieldObjectData = static_pointer_cast<SerializedObject>(diffData);

              IReflectable& childObj = field->getValue(object.get());
              SPtr<IReflectable> clonedObj = BinaryCloner::clone(&childObj, true);

              IDiff::applyDiff(clonedObj->getRTTI(),
                               clonedObj,
                               fieldObjectData,
                               objectMap,
                               diffCommands);

              DiffCommand command;
              command.field = genericField;
              command.type = DIFF_COMMAND_TYPE::kReflectable;
              command.object = clonedObj;

              diffCommands.push_back(command);
            }
              break;
            case SERIALIZABLE_FIELD_TYPE::kPlain:
            {
              SPtr<SerializedField>
                diffFieldData = static_pointer_cast<SerializedField>(diffData);

              if (diffFieldData->size > 0) {
                DiffCommand command;
                command.field = genericField;
                command.type = DIFF_COMMAND_TYPE::kPlain;
                command.value = diffFieldData->value;
                command.size = diffFieldData->size;

                diffCommands.push_back(command);
              }
            }
            break;
            case SERIALIZABLE_FIELD_TYPE::kDataBlock:
            {
              SPtr<SerializedDataBlock> 
                diffFieldData = static_pointer_cast<SerializedDataBlock>(diffData);

              DiffCommand command;
              command.field = genericField;
              command.type = DIFF_COMMAND_TYPE::kDataBlock;
              command.streamValue = diffFieldData->stream;
              command.value = nullptr;
              command.size = diffFieldData->size;

              diffCommands.push_back(command);
            }
              break;
          }
        }
      }
    }

    DiffCommand objEndCommand;
    objEndCommand.field = nullptr;
    objEndCommand.type = DIFF_COMMAND_TYPE::kObjectEnd;
    objEndCommand.object = object;

    diffCommands.push_back(objEndCommand);

    while (!rttiTypes.empty()) {
      rttiTypes.top()->onSerializationEnded(object.get(), dummyParams);
      rttiTypes.pop();
    }
  }
}
