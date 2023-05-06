print("[LUA] loading utils.lua")


function print_table (t)
    for k,v in pairs(t) do
      print(k, v)
      if type(v) == "table" then
        print_table(v)
      end
    end
  end
