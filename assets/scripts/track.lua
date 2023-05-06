print("[LUA] Loading main.lua")
require("utils")


local count = 1
local width = core.getWidth()
local height = core.getHeight()
local lastX, lastY = 0, 0


local lines = {}


function addBox(x,y,w,h)

     body = core.physics.newBody(0,x,y)
     shape = body:addBox(w,h)

end

function addCircle(x,y,r)

     circle = core.physics.newBody(0,x,y)
    circle:addCircle(25)

end

function load()
    core.log(0,"[LUA] call load main.lua")

     image = core.graphics.newImage("assets/images/zazaka.png")
     image2 = core.graphics.newImage("assets/images/zazaka.png")
     core.physics.setGravity(0,9.8)
    


    body = core.physics.newBody(1,0,0)
    body:addBox(10,height,5,height/2,0)

    body:addBox(width*10,10,width/2,height,0)
    local x =0
    local y = height-10
    local wh =600
    body:addLine( x, height-10, x+wh, y)
    x = x + wh
    y = y - 50
    body:addLine( x, height-10, x+wh, y)



    


    local x = 400
    local y = 100
    car = core.physics.newBody(0,x,y)
    local chassis = car:addBox(100,20)
    chassis:setDensity(0.1)


     wheel1 = core.physics.newBody(0,x-40,y+25)
    local sp1 = wheel1:addCircle(10)
    sp1:setFriction(0.8)
    sp1:setDensity(1.0)

     wheel2 = core.physics.newBody(0,x+40,y+25)
    local sp2 = wheel2:addCircle(10)
    sp2:setFriction(0.8)
    sp2:setDensity(1.0)

    local axisX=0.0
    local axisY=1.0



     joint1 = core.physics.newWheelJoint(car,wheel1,x-40,y+25,axisX,axisY)
     joint2 = core.physics.newWheelJoint(car,wheel2,x+40,y+25,axisX,axisY)

     local hertz = 4.0
     local dampingRatio = 0.7
     local omega = 2.0 * math.pi * hertz
 
     local mass = wheel1:getMass()
     
     
 
     local stiffness = mass * omega * omega
     local damping = 2.0 * mass * dampingRatio * omega
 
     joint1:setMotorEnabled(true)
     joint1:setMotorSpeed(0.0)
     joint1:setMaxMotorTorque(1900.0)
     joint1:setStiffness(stiffness)
     joint1:setDamping(damping)
     joint1:setLimitsEnabled(true)
     joint1:setLimits(-0.25,0.25)
 
     joint2:setMotorEnabled(true)
     joint2:setMotorSpeed(0.0)
     joint2:setMaxMotorTorque(1900.0)
     joint2:setStiffness(stiffness)
     joint2:setDamping(damping)
     joint2:setLimitsEnabled(true)
     joint2:setLimits(-0.25,0.25)

   --core.graphics.setCameraZoom(0.5)
    point ={}
    point.x=width/2
    point.y=height/2


   --joint1:setMotorSpeed(-20.0)
   --joint2:setMotorSpeed(-20.0)
   core.graphics.setCameraOrigin(width/2,height/2)

    lines = get_track()

    for i, line in ipairs(lines) do
        local x1, y1, x2, y2 = line[1], line[2], line[3], line[4]
        body:addLine(x1, y1, x2, y2)
      end







end
  
     


function close()

    core.log(0,"[LUA] call close main.lua")
end

function draw()
    --core.graphics.setCameraZoom(core.graphics.getCameraZoom()+core.mouse.getWheel())
core.graphics.setColor(1,1,1)
core.graphics.print("Hello World ",10,10)

core.graphics.draw(image,200,200,false,false)
core.graphics.setColor(1,0,0)
core.graphics.circle("line",300,200,5)

core.graphics.beginCamera()
     core.physics.draw()

     for i, line in ipairs(lines) do
        core.graphics.line(line[1], line[2], line[3], line[4])
      end

core.graphics.endCamera()

end

function update(dt)

         core.physics.update(1.0/60.0)

         if (core.mouse.press(0)) then
                local x, y = core.mouse.getWorldX(), core.mouse.getWorldY()
                if lastX == 0 then -- primeiro clique
                    lastX, lastY = x, y
                  else -- segundo clique
                    table.insert(lines, {lastX, lastY, x, y})
                    lastX, lastY = x, y
                  end
         end

         local car_x,car_y = car:getPosition()
         core.graphics.setCameraPosition(car_x,car_y)
         --core.graphics.setCameraPosition(point.x,point.y)

            if (core.keyboard.down("left")) then
                point.x = point.x -10
            end
            if (core.keyboard.down("right")) then
                point.x = point.x +10
            end


        if (core.keyboard.press("space")) then
            addBox(core.mouse.getWorldX(),core.mouse.getWorldY(),20,20) 
        end

        if (core.keyboard.press("s")) then
            local file = io.open("lines.lua", "w")
            file:write("return {\n")
            for i, line in ipairs(lines) do
              file:write(string.format("{%d, %d, %d, %d},\n", line[1], line[2], line[3], line[4]))
            end
            file:write("}")
            file:close()
        end

        if (core.keyboard.down("a")) then
            joint1:setMotorSpeed(-100.0)
            joint2:setMotorSpeed(-100.0)
            
        end
        if (core.keyboard.down("d")) then
            joint1:setMotorSpeed(80.0)
            joint2:setMotorSpeed(80.0)
        end

        if (core.keyboard.down("s")) then
            joint1:setMotorSpeed(0.0)
            joint2:setMotorSpeed(0.0)
        end
        

        --  if (core.mouse.press(1)) then
        --     addCircle(core.mouse.getX(),core.mouse.getY(),5)
        -- end

end

function ready()

  
end


function get_track()
    return {
        {-42, 345, 303, 352},
        {303, 352, 571, 411},
        {571, 411, 997, 438},
        {997, 438, 1451, 436},
        {1451, 436, 1535, 422},
        {1535, 422, 1575, 412},
        {1575, 412, 1605, 405},
        {1605, 405, 1775, 379},
        {1775, 379, 1914, 336},
        {1914, 336, 2047, 332},
        {2047, 332, 2105, 387},
        {2105, 387, 2226, 462},
        {2226, 462, 2374, 462},
        {2374, 462, 2544, 458},
        {2544, 458, 2724, 456},
        {2724, 456, 2922, 448},
        {2922, 448, 3478, 531},
        {3478, 531, 3717, 531},
        {3717, 531, 3852, 510},
        {3852, 510, 3888, 541},
        {3888, 541, 3969, 543},
        {3969, 543, 4067, 537},
        {4067, 537, 4150, 539},
        {4150, 539, 4246, 533},
        {4246, 533, 4359, 509},
        {4359, 509, 4435, 532},
        {4435, 532, 4541, 534},
        {4541, 534, 4711, 534},
        {4711, 534, 4861, 534},
        {4861, 534, 5001, 534},
        {5001, 534, 5275, 491},
        {5275, 491, 5560, 485},
        {5560, 485, 5800, 447},
        {5800, 447, 5974, 422},
        {5974, 422, 6296, 335},
        {6296, 335, 6630, 293},
        {6630, 293, 6914, 293},
        {6914, 293, 7144, 305},
        {7144, 305, 7340, 293},
        {7340, 293, 7593, 271},
        {7593, 271, 7843, 258},
        {7843, 258, 8043, 258},
        {8043, 258, 8153, 258},
        {8153, 258, 8666, 370},
        {8666, 370, 8838, 397},
        {8838, 397, 9098, 411},
        {9098, 411, 9320, 407},
        {9320, 407, 9632, 407},
        {9632, 407, 9912, 407},
        }
end