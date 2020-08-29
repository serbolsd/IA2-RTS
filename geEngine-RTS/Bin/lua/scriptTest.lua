--functions
--NormalizeVector(vector2)
--AddVectors(vector2, vector2)
--SubstracVectors(vector2, vector2)
--MultiplyVecForMagnitude(vector2, float)
--CheckVectorZero(vector2)
_this = player.new();
zeroVector = vector.new();
vect = vector.new();
newDir = vector.new();
function Update()
 -- _this.position = AddVectors(_this.position, _this.direction);
  if CheckVectorZero(_this.force) then
    _this.direction = zeroVector.vec
  else
    _this.direction = _this.force;
  end

  if CheckVectorZero(_this.direction) then
    newDir.vec = vect.vec
  else
    newDir.vec = _this.direction;
    newDir.vec = NormalizeVector(newDir.vec);
    speedForTime = _this.speed * _this.deltaTime
    newDir.vec = MultiplyVecForMagnitude(newDir.vec, speedForTime);
  end
  _this.direction = newDir.vec
  _this.position = AddVectors(_this.position, _this.direction);
  --return _this.position;
end

function getPos()
  --print (_this.position)
  --lel = _this.speed * _this.deltaTime;
  --print (lel)
  --lol = SubstracVectors(_this.position, vect.vec);
  return _this.position
end
--
function setPos(pos)--vector2
  _this.position = pos
end

function getForce()
  return _this.force; 
end

function setForce(force)--vector2
  _this.force = force
end

function getDirection()
  return _this.direction; 
end

function setDirection(dir)--vector2
  _this.direction = dir 
end

function setSpeed(speed)--float
  _this.speed = speed
end

function setDeltaTime(deltaTime)--float
  _this.deltaTime = deltaTime
end