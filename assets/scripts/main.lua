



function load()
   

demo = core.filesystem.load("assets/scripts/scene_1.lua")
demo:load()


end
  


function close()


end

function draw()

core.graphics.setColor(255,255,255)
    
 
core.graphics.beginCamera()
       


core.physics.draw()
 
demo:draw()


  

core.graphics.endCamera()

core.gui.label("Texte",20,20,100,20)
if core.gui.button("BUtton",20,50,100,20) then
      print("Button clicked")
end

core.graphics.drawFPS(5,5)

end

function update(dt)

         core.physics.update(1.0/60.0)
         demo:update(dt)




end


