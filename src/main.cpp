 

#define CORE_IMPLEMENTATION
#include "Core.hpp"

#define PHYSICS_IMPLEMENTATION
#include "Physics.hpp"

 //#define RAYGUI_IMPLEMENTATION
//#include "raygui.h"
  
void PrintLuaStack(lua_State *L)
{
    int top = lua_gettop(L);
    for (int i = 1; i <= top; i++)
    {
        int t = lua_type(L, i);
        switch (t)
        {
        case LUA_TSTRING:
            Log(LOG_INFO, "s:%s\n", lua_tostring(L, i));
            break;
        case LUA_TBOOLEAN:
            Log(LOG_INFO, "%s \n", lua_toboolean(L, i) ? "true" : "false");
            break;
        case LUA_TNUMBER:
            Log(LOG_INFO, "N: %g\n", lua_tonumber(L, i));
            break;
        default:
            Log(LOG_INFO, "N:%s\n", lua_typename(L, t));
            // PrintLuaTable(L);
            break;
        }
        printf(" ");
    }
    printf("\n");
}

void PrintLuaTable(lua_State *L)
{
    lua_pushnil(L);
    while (lua_next(L, -2) != 0)
    {
        // A chave está no índice -2 e o valor está no índice -1 da pilha
        if (lua_isstring(L, -2))
        {
            const char *key = lua_tostring(L, -2); // Lê a chave como uma string
            Log(LOG_INFO, "  %s", key);
        }
        lua_pop(L, 1); // Remove o valor, mas mantém a chave para a próxima iteração
    }
}

void PrintTopValue(lua_State *L)
{
    int n = lua_gettop(L); // Número de parâmetros
    for (int i = 1; i <= n; i++)
    {
        const char *tname = lua_typename(L, lua_type(L, i));
        Log(LOG_INFO, "  %d: %s", i, tname);
    }
}

int main(int argc, char **argv)
{
    std::string path = GetDirectoryPath(argv[0]);

    lua_State *L = luaL_newstate();
    luaL_openlibs(L);
    CreateCore(L);
    lua_getglobal(L, "core");
    if (!lua_isnil(L, -1))
    {
        lua_newtable(L);
        int i;
        for (i = 0; i < argc; i++)
        {
            lua_pushstring(L, argv[i]);
            lua_rawseti(L, -2, i + 1);
        }
        lua_setfield(L, -2, "argv");
    }
    lua_pop(L, 1);

 

    const char *script = R"lua(

RAD = math.pi / 180
DEG = 180 / math.pi

function max(a, b)
    return (a > b) and a or b
end

function min(a, b)
    return (a < b) and a or b
end

function lengthdir_x(length, direction)
    return length * math.cos(direction * RAD)
end

function lengthdir_y(length, direction)
    return length * math.sin(direction * RAD)
end

function point_distance(x1, y1, x2, y2)
    return math.sqrt((x1 - x2)^2 + (y1 - y2)^2)
end

function point_direction(x1, y1, x2, y2)
    return math.atan2(y2 - y1, x2 - x1) * DEG
end

function deg2rad(degrees)
    return degrees * RAD
end

function rad2deg(radians)
    return radians * DEG
end
-- Clamp a value between a minimum and maximum
function clamp(value, min, max)
return value < min and min or (value > max and max or value)
end

-- Linear interpolation between two values
function Lerp(start, finish, amount)
    return start + amount * (finish - start)
end

-- Normalize an angle to be in the range [-pi, pi]
function normalizeAngle(angle)
    while angle < -math.pi do
        angle = angle + math.pi * 2
    end
    while angle > math.pi do
        angle = angle - math.pi * 2
    end
    return angle
end

-- Calculate the angle between two points in degrees
function fget_angle(x1, y1, x2, y2)
    local dx = x2 - x1
    local dy = y2 - y1
    local angle

    if dx == 0 then
        return dy > 0 and 270 or 90
    end

    angle = math.atan(dy / dx) * 180.0 / math.pi

    return dx > 0 and -angle or -angle + 180
end


-- Calculate the x distance traveled with a given angle and speed
function get_distx(angle, speed)
    local a = angle * math.pi / 180.0
    return speed * math.cos(a)
end

-- Calculate the y distance traveled with a given angle and speed
function get_disty(angle, speed)
    local a = angle * math.pi / 180.0
    return speed * -math.sin(a)
end

function lerpAngleDegrees(a, b, lerpFactor)
    local result
    local diff = b - a
    if diff < -180 then
        -- lerp upwards past 360
        b = b + 360
        result = Lerp(a, b, lerpFactor)
        if result >= 360 then
            result = result - 360
        end
    elseif diff > 180 then
        -- lerp downwards past 0
        b = b - 360
        result = Lerp(a, b, lerpFactor)
        if result < 0 then
            result = result + 360
        end
    else
        -- straight lerp
        result = Lerp(a, b, lerpFactor)
    end

    return result
end

function lerpAngleRadians(a, b, lerpFactor)
    local result
    local diff = b - a
    if diff < -math.pi then
        -- lerp upwards past PI_TIMES_TWO
        b = b + PI_TIMES_TWO
        result = Lerp(a, b, lerpFactor)
        if result >= PI_TIMES_TWO then
            result = result - PI_TIMES_TWO
        end
    elseif diff > math.pi then
        -- lerp downwards past 0
        b = b - PI_TIMES_TWO
        result = Lerp(a, b, lerpFactor)
        if result < 0 then
            result = result + PI_TIMES_TWO
        end
    else
        -- straight lerp
        result = Lerp(a, b, lerpFactor)
    end

    return result
end

-- Calculates a Hermite interpolation between two values, given two tangents and an amount between 0 and 1
function Hermite(value1, tangent1, value2, tangent2, amount)
    local v1 = value1
    local v2 = value2
    local t1 = tangent1
    local t2 = tangent2
    local s = amount
    local result
    local sCubed = s * s * s
    local sSquared = s * s

    if amount == 0 then
        result = value1
    elseif amount == 1 then
        result = value2
    else
        result = (2 * v1 - 2 * v2 + t2 + t1) * sCubed +
                 (3 * v2 - 3 * v1 - 2 * t1 - t2) * sSquared +
                 t1 * s +
                 v1
    end

    return result
end


-- Calculate a smoothstep interpolation between two values
function SmoothStep(value1, value2, amount)
local result = clamp(amount, 0, 1)
result = Hermite(value1, 0, value2, 0, result)
return result
end
-- Calculate the angle between two points in degrees
function getAngle(x1, y1, x2, y2)
local a = math.atan2(y2 - y1, x2 - x1) * 180 / math.pi
return a < 0 and a + 360 or a
end

-- Linear interpolation between two floats
function floatLerp(value1, value2, amount)
return value1 + (value2 - value1) * amount
end

-- Return the sign of a value (-1, 0, or 1)
function sign(value)
    return value < 0 and -1 or (value > 0 and 1 or 0)
end



-- Repeats a value t, with a specified length
function repeat_value(t, length)
    return clamp(t - math.floor(t / length) * length, 0, length)
end

-- Ping-pongs a value t, with a specified length
function ping_pong(t, length)
    t = repeat_value(t, length * 2)
    return length - math.abs(t - length)
end


  function boot()

   -- package.path = "?.lua;"
    package.path = package.path .. ";assets/?.lua;assets/?/init.lua;assets/scripts/?.lua;assets/scripts/?/init.lua;assets/lua/?.lua;assets/lua/?/init.lua;../?.lua;../assets/?.lua;../assets/?/init.lua;../lua/?.lua;../lua/?/init.lua;../scripts/?.lua;../scripts/?/init.lua"


    table.insert(package.searchers, 1, function(modname)
      modname = modname:gsub("%.", "/")
      print("[LUA] Load Modname: " .. modname)
      for x in package.path:gmatch("[^;]+") do
        local file = x:gsub("?", modname)
        if core.filesystem.exists(file) then
            return assert(load(core.filesystem.read(file), "=" .. file))
        end
      --  print("file: " .. file)
      end
    end)
  
    local coreConfig = 
    {
		title = "DjokerSoft Core2D@",
		version = core.getVersion(),
		window = 
        {
			width = 800,
			height = 600,
            fps = 60,
            title ="Core2D by Djoker",
            vsync = true,
            fullscreen = false,
            borderless = false,
            resizable = false,
        }
    }
            
  core.setup(coreConfig.window.width, coreConfig.window.height, coreConfig.window.fps, 
  coreConfig.window.title, coreConfig.window.vsync, coreConfig.window.fullscreen, coreConfig.window.borderless, coreConfig.window.resizable)

  --print_table(coreConfig)
  core.init()
    end

function errhand(msg)
    core.log(2,msg)
end


Color = {}
Color.__index = Color

function Color.new(r, g, b, a)
    local color = {}
    setmetatable(color, Color)
    color.r = r or 255
    color.g = g or 255
    color.b = b or 255
    color.a = a or 255
    return color
end

LightGray = Color.new(200, 200, 200, 255)
Gray = Color.new(130, 130, 130, 255)
DarkGray = Color.new(80, 80, 80, 255)
Yellow = Color.new(253, 249, 0, 255)
Gold = Color.new(255, 203, 0, 255)
Orange = Color.new(255, 161, 0, 255)
Pink = Color.new(255, 109, 194, 255)
Red = Color.new(230, 41, 55, 255)
Maroon = Color.new(190, 33, 55, 255)
Green = Color.new(0, 228, 48, 255)
Lime = Color.new(0, 158, 47, 255)
DarkGreen = Color.new(0, 117, 44, 255)
SkyBlue = Color.new(102, 191, 255, 255)
Blue = Color.new(0, 121, 241, 255)
DarkBlue = Color.new(0, 82, 172, 255)
Purple = Color.new(200, 122, 255, 255)
Violet = Color.new(135, 60, 190, 255)
DarkPurple = Color.new(112, 31, 126, 255)
Beige = Color.new(211, 176, 131, 255)
Brown = Color.new(127, 106, 79, 255)
DarkBrown = Color.new(76, 63, 47, 255)
White = Color.new(255, 255, 255, 255)
Black = Color.new(0, 0, 0, 255)
Blank = Color.new(0, 0, 0, 0)
Magenta = Color.new(255, 0, 255, 255)
RayWhite = Color.new(245, 245, 245, 255)
SkyBlueLight = Color.new(102, 204, 255, 255)
DarkBlueLight = Color.new(51, 204, 255, 255)
BlueLight = Color.new(0, 204, 255, 255)
PurpleLight = Color.new(204, 51, 255, 255)
RedLight = Color.new(255, 102, 102, 255)
YellowLight = Color.new(255, 255, 204, 255)
GreenLight = Color.new(153, 255, 153, 255)
OrangeLight = Color.new(255, 204, 153, 255)
BeigeLight = Color.new(255, 255, 204, 255)
BrownLight = Color.new(153, 102, 51, 255)
DarkGreenLight = Color.new(51, 204, 51, 255)
MagentaLight = Color.new(255, 102, 255, 255)
GrayLight = Color.new(179, 179, 179, 255)
GrayDark = Color.new(102, 102, 102, 255)
GoldLight = Color.new(255, 204, 51, 255)
GoldDark = Color.new(255, 102, 0, 255)




     xpcall(boot, errhand)

    )lua";

    int result = luaL_dostring(L, script);
    if (result != LUA_OK)
    {
        
        Log(LOG_ERROR, "running Lua code: %s", lua_tostring(L, -1));
    }

    CoreLoad(argc, argv);
    LoopCore();
    CloseCore(L);
    return 0;
}