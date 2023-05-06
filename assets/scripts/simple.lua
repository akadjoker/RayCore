
local width = core.getWidth()
local height = core.getHeight()


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
     
     core.physics.setGravity(0,9.8)
    


    body = core.physics.newBody(1,0,0)
    body:addBox(10,height,5,height/2,0)

    body:addBox(width*10,10,width/2,height,0)


    


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



 



end
  
     


function close()


end

function draw()

core.graphics.setColor(1,1,1)


core.graphics.beginCamera()
     core.physics.draw()


core.graphics.endCamera()

end

function update(dt)

         core.physics.update(1.0/60.0)


end

function ready()

  
end
