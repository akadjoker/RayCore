#ifndef CORE_H
#define CORE_H

#include <string>
#include <vector>
#include <lua.hpp>
#include "raylib.h"
#include "raymath.h"
#include "rlgl.h"
#include "raygui.h"

#define Push_int(L, value) lua_pushinteger(L, value)
#define Push_float(L, value) lua_pushnumber(L, value);
#define Push_double(L, value) lua_pushnumber(L, value);
#define Push_bool(L, value) lua_pushboolean(L, value)
#define Push_string(L, value) lua_pushstring(L, value)

#define Get_ptr (void *)luaL_checkinteger
#define Get_int (int)luaL_checkinteger
#define Get_bool (bool)lua_toboolean
#define Get_unsigned (unsigned)luaL_checkinteger
#define Get_char (char)luaL_checkinteger
#define Get_float (float)luaL_checknumber
#define Get_double luaL_checknumber
#define Get_string luaL_checkstring

#define CONSOLE_COLOR_RESET "\033[0m"
#define CONSOLE_COLOR_GREEN "\033[1;32m"
#define CONSOLE_COLOR_RED "\033[1;31m"
#define CONSOLE_COLOR_PURPLE "\033[1;35m"
#define CONSOLE_COLOR_CYAN "\033[0;36m"
#define CORE_VERSION "0.0.1"

inline void Log(int severity, const char *fmt, ...)
{
    /* Determine strings for the type and colour */
    const char *type;
    const char *color;
    switch (severity)
    {
    case LOG_INFO:
        type = "info";
        color = CONSOLE_COLOR_GREEN;
        break;
    case LOG_ERROR:
        type = "error";
        color = CONSOLE_COLOR_RED;
        break;
    case LOG_WARNING:
        type = "warning";
        color = CONSOLE_COLOR_PURPLE;
        break;
    default:
        break; /* Unreachable */
    }

    /* Obtain the current date and time */
    time_t rawTime;
    struct tm *timeInfo;
    char timeBuffer[80];

    time(&rawTime);
    timeInfo = localtime(&rawTime);

    strftime(timeBuffer, sizeof(timeBuffer), "[%H:%M:%S]", timeInfo);

    /* Format for printing to the console (with colours) */
    char consoleFormat[1024];
    snprintf(consoleFormat, 1024, "%s%s %s%s%s: %s\n", CONSOLE_COLOR_CYAN,
             timeBuffer, color, type, CONSOLE_COLOR_RESET, fmt);

    va_list argptr;

    /* Print to the console */
    va_start(argptr, fmt);
    vprintf(consoleFormat, argptr);
    va_end(argptr);
}

void CloseCore(lua_State *L);
void CreateCore(lua_State *L);
void LoopCore();
void CoreLoad(int argc, char **argv);
#endif

#ifdef CORE_IMPLEMENTATION

#include "Physics.hpp"
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <map>

Rectangle iRect(int x, int y, int w, int h)
{
    Rectangle r;
    r.x = x;
    r.y = y;
    r.width = w;
    r.height = h;
    return r;
}

Rectangle Rect(float x = 0, float y = 0, float w = 0, float h = 0)
{
    Rectangle r;
    r.x = x;
    r.y = y;
    r.width = w;
    r.height = h;
    return r;
}

Vector2 Vec2(float x = 0, float y = 0)
{
    Vector2 v;
    v.x = x;
    v.y = y;
    return v;
}

Vector2 iVec2(int x = 0, int y = 0)
{
    Vector2 v;
    v.x = x;
    v.y = y;
    return v;
}

class Graph
{
public:
    int id;
    Texture2D texture;
    Rectangle clip;
};

class Assets
{
public:
    bool hasTexture(const std::string &key)
    {
        auto it = textures.find(key);
        if (it != textures.end())
        {
            return true;
        }
        return false;
    }
    bool hasGraph(int id)
    {
        auto it = graphics.find(id);
        if (it != graphics.end())
        {
            return true;
        }
        return false;
    }

    Graph getGraph(int id)
    {
        return graphics[id];
    }

    bool addGraph(const std::string &filename, int id)
    {
        if (FileExists(filename.c_str()) == false)
        {
            Log(LOG_ERROR, "File not found %s", filename.c_str());
            return false;
        }
        Graph graph;
        std::string key = GetFileNameWithoutExt(filename.c_str());

        if (hasTexture(key))
        {
            Log(LOG_WARNING, "Texture already loaded %s", filename.c_str());
            graph.texture = textures[key];
        }
        else
        {
            graph.texture = LoadTexture(filename.c_str());
            textures[key] = graph.texture;
        }
        graph.clip.x = 0;
        graph.clip.y = 0;
        graph.clip.width = graph.texture.width;
        graph.clip.height = graph.texture.height;

        graph.id = id;
        graphics[graph.id] = graph;
        return true;
    }
    bool addGrid(const std::string &filename, int id, int framesCount, int rows, int columns)
    {

        if (FileExists(filename.c_str()) == false)
        {
            Log(LOG_ERROR, "File not found %s", filename.c_str());
            return false;
        }
        Graph graph;
        std::string key = GetFileNameWithoutExt(filename.c_str());

        if (hasTexture(key))
        {
            Log(LOG_WARNING, "Texture already loaded %s", filename.c_str());
            graph.texture = textures[key];
        }
        else
        {
            graph.texture = LoadTexture(filename.c_str());
            textures[key] = graph.texture;
        }
        graph.clip.x = 0;
        graph.clip.y = 0;
        graph.clip.width = graph.texture.width;
        graph.clip.height = graph.texture.height;
        graph.id = id;
        graphics[graph.id] = graph;

        Texture texture;

        int imageWidth = (int)graph.clip.width;
        int imageHeight = (int)graph.clip.height;

        int frame = 0;
        int startId = id + 1;
        for (int i = 0; i < framesCount; i++)
        {

            float width = imageWidth / columns;
            float height = imageHeight / rows;
            float x = (frame % columns) * width;
            float y = (frame / columns) * height;
            addSubGraph(key, startId, x, y, width, height);
            startId++;
            frame++;
        }

        return true;
    }
    bool addGraph(const std::string &filename, int id, float x, float y, float w, float h)
    {
        if (FileExists(filename.c_str()) == false)
        {
            Log(LOG_ERROR, "File not found %s", filename.c_str());
            return false;
        }
        Graph graph;
        std::string key = GetFileNameWithoutExt(filename.c_str());

        if (hasTexture(key))
        {
            Log(LOG_WARNING, "Texture already loaded %s", filename.c_str());
            graph.texture = textures[key];
        }
        else
        {
            graph.texture = LoadTexture(filename.c_str());
            textures[key] = graph.texture;
        }
        graph.clip.x = x;
        graph.clip.y = y;
        graph.clip.width = w;
        graph.clip.height = h;
        graph.id = id;
        graphics[graph.id] = graph;
        return true;
    }

    bool addSubGraph(const std::string &key, int id, float x, float y, float w, float h)
    {
        Graph graph;
        graph.texture = textures[key];
        graph.clip.x = x;
        graph.clip.y = y;
        graph.clip.width = w;
        graph.clip.height = h;
        graph.id = id;
        graphics[graph.id] = graph;
        return true;
    }
    void clear()
    {
        for (auto &graph : textures)
        {
            Log(LOG_WARNING, " Unload image  %s ", graph.first.c_str());
            UnloadTexture(graph.second);
        }
        textures.clear();
    }
    std::map<std::string, Texture2D> textures;
    std::map<int, Graph> graphics;
};

static Color _color;
static Color _backgroundColor;
static Camera2D _camera;
static int screenWidth = 800;
static int screenHeight = 600;
static int screenFps = 60;
static std::string screenTitle;
static bool screenVsync = false;
static bool screenFullscreen = false;
static bool screenBorderless = false;
static bool screenResizable = true;
static bool screenInit = false;
static std::map<std::string, int> _keys;
static Assets _assets;

namespace nGraphics
{

#define FIX_ARTIFACTS_BY_STRECHING_TEXEL true

    typedef struct rVertex
    {
        float x, y, z;
        Color col;
        float tx, ty;

    } rVertex;

    typedef struct rQuad
    {
        rVertex v[4];
        Texture2D tex;
        int blend;
    } rQuad;

    void RenderQuadMatrix(const rQuad *quad)
    {

        rlCheckRenderBatchLimit(4); // Make sure there is enough free space on the batch buffer
        rlSetTexture(quad->tex.id);

        rlBegin(RL_QUADS);

        Color a = quad->v[1].col;
        Color b = quad->v[0].col;
        Color c = quad->v[3].col;
        Color d = quad->v[2].col;

        rlNormal3f(0.0f, 0.0f, 1.0f);

        rlColor4ub(a.r, a.g, a.b, a.a);
        rlTexCoord2f(quad->v[1].tx, quad->v[1].ty);
        rlVertex3f(quad->v[1].x, quad->v[1].y, quad->v[1].z);

        rlColor4ub(b.r, b.g, b.b, b.a);
        rlTexCoord2f(quad->v[0].tx, quad->v[0].ty);
        rlVertex3f(quad->v[0].x, quad->v[0].y, quad->v[0].z);

        rlColor4ub(c.r, c.g, c.b, c.a);
        rlTexCoord2f(quad->v[3].tx, quad->v[3].ty);
        rlVertex3f(quad->v[3].x, quad->v[3].y, quad->v[3].z);

        rlColor4ub(d.r, d.g, d.b, d.a);
        rlTexCoord2f(quad->v[2].tx, quad->v[2].ty);
        rlVertex3f(quad->v[2].x, quad->v[2].y, quad->v[2].z);

        rlEnd();
    }

    void RenderClipFlip(Texture2D texture, int width, int height, Rectangle clip, bool flipX, bool flipY, Color color, const Matrix *mat, int blend)
    {

        rQuad quad;
        quad.tex = texture;
        quad.blend = blend;

        int widthTex = texture.width;
        int heightTex = texture.height;

        float left;
        float right;
        float top;
        float bottom;

        if (FIX_ARTIFACTS_BY_STRECHING_TEXEL)
        {
            left = (2 * clip.x + 1) / (2 * widthTex);
            right = left + (clip.width * 2 - 2) / (2 * widthTex);
            top = (2 * clip.y + 1) / (2 * heightTex);
            bottom = top + (clip.height * 2 - 2) / (2 * heightTex);
        }
        else
        {
            left = clip.x / widthTex;
            right = (clip.x + clip.width) / widthTex;
            top = clip.y / heightTex;
            bottom = (clip.y + clip.height) / heightTex;
        }

        if (flipX)
        {
            float tmp = left;
            left = right;
            right = tmp;
        }

        if (flipY)
        {
            float tmp = top;
            top = bottom;
            bottom = tmp;
        }

        float TempX1 = 0;
        float TempY1 = 0;
        float TempX2 = width;
        float TempY2 = height;

        quad.v[1].x = TempX1;
        quad.v[1].y = TempY1;
        quad.v[1].tx = left;
        quad.v[1].ty = top;

        quad.v[0].x = TempX1;
        quad.v[0].y = TempY2;
        quad.v[0].tx = left;
        quad.v[0].ty = bottom;

        quad.v[3].x = TempX2;
        quad.v[3].y = TempY2;
        quad.v[3].tx = right;
        quad.v[3].ty = bottom;

        quad.v[2].x = TempX2;
        quad.v[2].y = TempY1;
        quad.v[2].tx = right;
        quad.v[2].ty = top;

        quad.v[0].z = quad.v[1].z = quad.v[2].z = quad.v[3].z = 0.0f;
        quad.v[0].col = quad.v[1].col = quad.v[2].col = quad.v[3].col = color;

        for (int i = 0; i < 4; i++)
        {

            float x = quad.v[i].x;
            float y = quad.v[i].y;
            float z = quad.v[i].z;

            quad.v[i].x = mat->m0 * x + mat->m4 * y + mat->m8 * z + mat->m12;
            quad.v[i].y = mat->m1 * x + mat->m5 * y + mat->m9 * z + mat->m13;
            quad.v[i].z = mat->m2 * x + mat->m6 * y + mat->m10 * z + mat->m14;
        }

        RenderQuadMatrix(&quad);
    }

    void RenderClipFlip(Texture2D texture, float x, float y, int width, int height, Rectangle clip, bool flipX, bool flipY, Color color, int blend)
    {

        rQuad quad;
        quad.tex = texture;
        quad.blend = blend;

        int widthTex = texture.width;
        int heightTex = texture.height;

        float left;
        float right;
        float top;
        float bottom;

        if (FIX_ARTIFACTS_BY_STRECHING_TEXEL)
        {
            left = (2 * clip.x + 1) / (2 * widthTex);
            right = left + (clip.width * 2 - 2) / (2 * widthTex);
            top = (2 * clip.y + 1) / (2 * heightTex);
            bottom = top + (clip.height * 2 - 2) / (2 * heightTex);
        }
        else
        {
            left = clip.x / widthTex;
            right = (clip.x + clip.width) / widthTex;
            top = clip.y / heightTex;
            bottom = (clip.y + clip.height) / heightTex;
        }

        if (flipX)
        {
            float tmp = left;
            left = right;
            right = tmp;
        }

        if (flipY)
        {
            float tmp = top;
            top = bottom;
            bottom = tmp;
        }

        float TempX1 = x;
        float TempY1 = y;
        float TempX2 = x + width;
        float TempY2 = y + height;

        quad.v[1].x = TempX1;
        quad.v[1].y = TempY1;
        quad.v[1].tx = left;
        quad.v[1].ty = top;

        quad.v[0].x = TempX1;
        quad.v[0].y = TempY2;
        quad.v[0].tx = left;
        quad.v[0].ty = bottom;

        quad.v[3].x = TempX2;
        quad.v[3].y = TempY2;
        quad.v[3].tx = right;
        quad.v[3].ty = bottom;

        quad.v[2].x = TempX2;
        quad.v[2].y = TempY1;
        quad.v[2].tx = right;
        quad.v[2].ty = top;

        quad.v[0].z = quad.v[1].z = quad.v[2].z = quad.v[3].z = 0.0f;
        quad.v[0].col = quad.v[1].col = quad.v[2].col = quad.v[3].col = color;

        RenderQuadMatrix(&quad);
    }

    static Matrix e = MatrixIdentity();
    Matrix setTransformation(float x, float y, float angle, float sx, float sy, float ox, float oy, float kx, float ky)
    {
        // Matrix e = MatrixIdentity();
        float c = cosf(angle), s = sinf(angle);
        // matrix multiplication carried out on paper:
        // |1     x| |c -s    | |sx       | | 1 ky    | |1     -ox|
        // |  1   y| |s  c    | |   sy    | |kx  1    | |  1   -oy|
        // |    1  | |     1  | |      1  | |      1  | |    1    |
        // |      1| |       1| |        1| |        1| |       1 |
        //   move      rotate      scale       skew       origin
        e.m10 = e.m15 = 1.0f;
        e.m0 = c * sx - ky * s * sy; // = a
        e.m1 = s * sx + ky * c * sy; // = b
        e.m4 = kx * c * sx - s * sy; // = c
        e.m5 = kx * s * sx + c * sy; // = d
        e.m12 = x - ox * e.m0 - oy * e.m4;
        e.m13 = y - ox * e.m1 - oy * e.m5;
        return e;
    }

    static int core_graphics_setBackgroundColor(lua_State *L)
    {
        if (lua_gettop(L) < 3)
        {
            return luaL_error(L, "setBackgroundColor function requires 3/4 arguments");
        }

        int r = Get_int(L, 1);
        int g = Get_int(L, 2);
        int b = Get_int(L, 3);

        if (lua_gettop(L) == 4)
        {
            _backgroundColor.a = Get_int(L, 4);
        }

        _backgroundColor.r = r;
        _backgroundColor.g = g;
        _backgroundColor.b = b;

        return 0;
    }
    static int core_graphics_begin_camera(lua_State *L)
    {
        (void)L;
        BeginMode2D(_camera);
        return 0;
    }
    static int core_graphics_end_camera(lua_State *L)
    {
        (void)L;
        EndMode2D();
        return 0;
    }
    static int core_graphics_getWidth(lua_State *L)
    {
        lua_pushnumber(L, GetScreenWidth());
        return 1;
    }

    static int core_graphics_getHeight(lua_State *L)
    {
        lua_pushnumber(L, GetScreenHeight());
        return 1;
    }

    static int core_graphics_print(lua_State *L)
    {
        if (lua_gettop(L) < 3)
        {
            return luaL_error(L, "print function requires 3 arguments");
        }

        const char *text = luaL_checkstring(L, 1);
        float x = (float)luaL_checknumber(L, 2);
        float y = (float)luaL_checknumber(L, 3);
        int size = 20;
        if (lua_gettop(L) == 4)
        {
            size = (int)luaL_checknumber(L, 4);
        }
        DrawText(text, x, y, size, _color);
        return 0;
    }

    static int core_graphics_drawFps(lua_State *L)
    {
        if (lua_gettop(L) < 2)
        {
            return luaL_error(L, "drawFps function requires 2 arguments");
        }

        int x = Get_int(L, 1);
        int y = Get_int(L, 2);

        DrawFPS(x, y);
        return 0;
    }
    static int core_graphics_rectangle(lua_State *L)
    {
        if (lua_gettop(L) < 5)
        {
            return luaL_error(L, "rectangle function requires 5 arguments");
        }

        const char *fill = luaL_checkstring(L, 1);
        float x = (float)luaL_checknumber(L, 2);
        float y = (float)luaL_checknumber(L, 3);
        float width = (float)luaL_checknumber(L, 4);
        float height = (float)luaL_checknumber(L, 5);
        if (strcmp(fill, "fill") == 0)
        {
            DrawRectangle(x, y, width, height, _color);
        }
        else
        {
            DrawRectangleLines(x, y, width, height, _color);
        }
        return 0;
    }

    static int core_graphics_circle(lua_State *L)
    {
        if (lua_gettop(L) < 4)
        {
            return luaL_error(L, "circle function requires 4 arguments");
        }

        const char *fill = luaL_checkstring(L, 1);
        float x = (float)luaL_checknumber(L, 2);
        float y = (float)luaL_checknumber(L, 3);
        float radius = (float)luaL_checknumber(L, 4);
        if (strcmp(fill, "fill") == 0)
        {
            DrawCircle(x, y, radius, _color);
        }
        else
        {
            DrawCircleLines(x, y, radius, _color);
        }
        return 0;
    }

    static int core_graphics_line(lua_State *L)
    {
        if (lua_gettop(L) < 4)
        {
            return luaL_error(L, "line function requires 4 arguments");
        }

        float x1 = (float)luaL_checknumber(L, 1);
        float y1 = (float)luaL_checknumber(L, 2);
        float x2 = (float)luaL_checknumber(L, 3);
        float y2 = (float)luaL_checknumber(L, 4);
        DrawLine(x1, y1, x2, y2, _color);
        return 0;
    }

    static int core_graphics_setColor(lua_State *L)
    {
        if (lua_gettop(L) < 3)
        {
            return luaL_error(L, "setColor function requires 3 arguments");
        }

        int r = Get_int(L, 1);
        int g = Get_int(L, 2);
        int b = Get_int(L, 3);

        if (lua_gettop(L) == 4)
        {
            _color.a = Get_int(L, 4);
        }

        _color.r = r;
        _color.g = g;
        _color.b = b;

        return 0;
    }
    // 40201  32
    static Matrix mat;
    static Rectangle source;

    static int core_graphics_draw(lua_State *L)
    {

        if (lua_gettop(L) < 3)
        {
            return luaL_error(L, "draw function requires 3+ arguments");
        }

        Texture2D *image_ptr = (Texture2D *)luaL_checkudata(L, 1, "Image");
        if (image_ptr == nullptr)
        {
            return luaL_error(L, "Invalid Image object");
        }
        Texture2D image = (*image_ptr);

        Rectangle *quad_ptr = (Rectangle *)luaL_testudata(L, 2, "Quad");
        float x = 0;
        float y = 0;
        float angle = 0;
        float sx = 1;
        float sy = 1;
        float ox = 0;
        float oy = 0;
        float kx = 0;
        float ky = 0;
        bool flipX = false;
        bool flipY = false;

        //   Log(LOG_INFO,"top: %d", lua_gettop(L));

        if (quad_ptr != NULL)
        {

            if (quad_ptr == nullptr)
            {
                return luaL_error(L, "Invalid Quad object");
            }
            Rectangle source = (*(quad_ptr));

            x = (float)luaL_checknumber(L, 3);
            y = (float)luaL_checknumber(L, 4);

            if (lua_gettop(L) == 6)
            {
                flipX = lua_toboolean(L, 5);
                flipY = lua_toboolean(L, 6);
            }
            else

                if (lua_gettop(L) > 4)
            {
                if (lua_gettop(L) < 11)
                {
                    return luaL_error(L, "draw image quad function requires 11 arguments");
                }
                angle = (float)luaL_checknumber(L, 5);
                sx = (float)luaL_checknumber(L, 6);
                sy = (float)luaL_checknumber(L, 7);
                ox = (float)luaL_checknumber(L, 8);
                oy = (float)luaL_checknumber(L, 9);

                kx = (float)luaL_checknumber(L, 10);
                ky = (float)luaL_checknumber(L, 11);

                if (lua_gettop(L) == 13)
                {
                    flipX = lua_toboolean(L, 12);
                    flipY = lua_toboolean(L, 13);
                }

                mat = setTransformation(x, y, angle, sx, sy, ox, oy, kx, ky);
                RenderClipFlip(image, source.width, source.height, source, flipX, flipY, _color, &mat, 0);

                return 0;
            }

            DrawTextureRec(image, source, (Vector2){x, y}, _color);
        }
        else
        {
            x = (float)luaL_checknumber(L, 2);
            y = (float)luaL_checknumber(L, 3);

            if (lua_gettop(L) == 5)
            {
                flipX = lua_toboolean(L, 4);
                flipY = lua_toboolean(L, 5);
            }
            else

                if (lua_gettop(L) > 3)
            {
                if (lua_gettop(L) < 10)
                {
                    return luaL_error(L, "draw image function requires 10 arguments");
                }
                angle = (float)luaL_checknumber(L, 4);
                sx = (float)luaL_checknumber(L, 5);
                sy = (float)luaL_checknumber(L, 6);
                ox = (float)luaL_checknumber(L, 7);
                oy = (float)luaL_checknumber(L, 8);
                kx = (float)luaL_checknumber(L, 9);
                ky = (float)luaL_checknumber(L, 10);
                if (lua_gettop(L) == 12)
                {
                    flipX = lua_toboolean(L, 11);
                    flipY = lua_toboolean(L, 12);
                }

                // Rectangle source;
                source.x = 0;
                source.y = 0;
                source.width = image.width;
                source.height = image.height;
                mat = setTransformation(x, y, angle, sx, sy, ox, oy, kx, ky);
                RenderClipFlip(image, source.width, source.height, source, false, false, _color, &mat, 0);

                return 0;
            }
            source.x = 0;
            source.y = 0;
            source.width = image.width;
            source.height = image.height;
            RenderClipFlip(image, x, y, source.width, source.height, source, flipX, flipY, _color, 0);
        }

        return 0;
    }

    static int core_graphics_draw_graph(lua_State *L)
    {

        if (lua_gettop(L) < 3)
        {
            return luaL_error(L, "draw function requires 3+ arguments");
        }

        int id = luaL_checkinteger(L, 1);
        Graph graph = _assets.getGraph(id);
        float x = 0;
        float y = 0;
        float angle = 0;
        float sx = 1;
        float sy = 1;
        float ox = 0;
        float oy = 0;
        float kx = 0;
        float ky = 0;
        bool flipX = false;
        bool flipY = false;

        x = (float)luaL_checknumber(L, 2);
        y = (float)luaL_checknumber(L, 3);

        if (lua_gettop(L) == 5)
        {
            flipX = lua_toboolean(L, 4);
            flipY = lua_toboolean(L, 5);
        }
        else

            if (lua_gettop(L) > 5)
        {
            if (lua_gettop(L) < 12)
            {
                return luaL_error(L, "draw graph quad function requires 12 arguments");
            }
            angle = (float)luaL_checknumber(L, 6);
            sx = (float)luaL_checknumber(L, 7);
            sy = (float)luaL_checknumber(L, 8);
            ox = (float)luaL_checknumber(L, 9);
            oy = (float)luaL_checknumber(L, 10);

            kx = (float)luaL_checknumber(L, 11);
            ky = (float)luaL_checknumber(L, 12);

            if (lua_gettop(L) == 14)
            {
                flipX = lua_toboolean(L, 13);
                flipY = lua_toboolean(L, 14);
            }

            mat = setTransformation(x, y, angle, sx, sy, ox, oy, kx, ky);
            RenderClipFlip(graph.texture, graph.clip.width, graph.clip.height, graph.clip, flipX, flipY, _color, &mat, 0);

            return 0;
        }

        DrawTextureRec(graph.texture, graph.clip, (Vector2){x, y}, _color);
        return 0;
    }
    static int core_graphics_point(lua_State *L)
    {
        if (lua_gettop(L) < 2)
        {
            return luaL_error(L, "point function requires 2 arguments");
        }

        float x = (float)luaL_checknumber(L, 1);
        float y = (float)luaL_checknumber(L, 2);

        DrawPixel(x, y, _color);

        return 0;
    }

    static int core_graphics_clear(lua_State *L)
    {

        (void)L;

        ClearBackground(_backgroundColor);

        return 0;
    }

    static int core_graphics_begin(lua_State *L)
    {
        (void)L;
        return 0;
    }
    static int core_graphics_end(lua_State *L)
    {
        (void)L;

        return 0;
    }

    static int core_graphics_getFont(lua_State *L)
    {
        Font *userdata = (Font *)lua_newuserdata(L, sizeof(Font));
        *userdata = GetFontDefault();
        luaL_getmetatable(L, "Font");
        lua_setmetatable(L, -2);
        return 1;
    }

    static int core_graphics_push(lua_State *L)
    {
        (void)L;

        rlPushMatrix();

        return 0;
    }

    static int core_graphics_pop(lua_State *L)
    {
        (void)L;
        rlPopMatrix();
        return 0;
    }

    static int core_graphics_translate(lua_State *L)
    {
        if (lua_gettop(L) < 2)
        {
            return luaL_error(L, "translate function requires 2 arguments");
        }

        float x = (float)luaL_checknumber(L, 1);
        float y = (float)luaL_checknumber(L, 2);

        rlTranslatef(x, y, 0);
        return 0;
    }

    static int core_graphics_rotate(lua_State *L)
    {
        if (lua_gettop(L) < 1)
        {
            return luaL_error(L, "rotate function requires 1 argument");
        }

        float angle = (float)luaL_checknumber(L, 1);

        rlRotatef(angle, 0, 0, 1);
        return 0;
    }

    static int core_graphics_scale(lua_State *L)
    {
        if (lua_gettop(L) < 2)
        {
            return luaL_error(L, "scale function requires 2 arguments");
        }

        float x = (float)luaL_checknumber(L, 1);
        float y = (float)luaL_checknumber(L, 2);

        rlScalef(x, y, 1);
        return 0;
    }

    static int core_graphics_shear(lua_State *L)
    {
        if (lua_gettop(L) < 2)
        {
            return luaL_error(L, "shear function requires 2 arguments");
        }

        float x = (float)luaL_checknumber(L, 1);
        float y = (float)luaL_checknumber(L, 2);

        return 0;
    }
    static int core_graphics_get_camera_offset(lua_State *L)
    {

        lua_pushnumber(L, _camera.offset.x);
        lua_pushnumber(L, _camera.offset.y);

        return 2;
    }

    static int core_graphics_set_camera_offset(lua_State *L)
    {

        if (lua_gettop(L) < 2)
        {
            return luaL_error(L, "set_camera_offet function requires 2 arguments");
        }

        float x = (float)luaL_checknumber(L, 1);
        float y = (float)luaL_checknumber(L, 2);

        _camera.offset.x = x;
        _camera.offset.y = y;

        return 0;
    }
    static int core_graphics_get_camera_rotation(lua_State *L)
    {

        lua_pushnumber(L, _camera.rotation);

        return 1;
    }

    static int core_graphics_set_camera_rotation(lua_State *L)
    {

        if (lua_gettop(L) < 1)
        {
            return luaL_error(L, "set_camera_rotation function requires 1 arguments");
        }

        float rotation = (float)luaL_checknumber(L, 1);

        _camera.rotation = rotation;

        return 0;
    }

    static int core_graphics_get_camera_zoom(lua_State *L)
    {

        lua_pushnumber(L, _camera.zoom);

        return 1;
    }

    static int core_graphics_set_camera_zoom(lua_State *L)
    {

        if (lua_gettop(L) < 1)
        {
            return luaL_error(L, "set_camera_zoom function requires 1 arguments");
        }

        float zoom = (float)luaL_checknumber(L, 1);

        _camera.zoom = zoom;

        return 0;
    }

    static int core_graphics_set_camera_target(lua_State *L)
    {

        if (lua_gettop(L) < 2)
        {
            return luaL_error(L, "set_camera_target function requires 2 arguments");
        }

        float x = (float)luaL_checknumber(L, 1);
        float y = (float)luaL_checknumber(L, 2);

        _camera.target.x = x;
        _camera.target.y = y;

        return 0;
    }
    static int core_graphics_get_camera_target(lua_State *L)
    {

        lua_pushnumber(L, _camera.target.x);
        lua_pushnumber(L, _camera.target.y);

        return 2;
    }

}

namespace nQuad
{

    static int newQuad(lua_State *L)
    {

        float x = luaL_checknumber(L, 1);
        float y = luaL_checknumber(L, 2);
        float width = luaL_checknumber(L, 3);
        float height = luaL_checknumber(L, 4);
        Rectangle *quad_ptr = (Rectangle *)lua_newuserdata(L, sizeof(Rectangle));
        quad_ptr->x = x;
        quad_ptr->y = y;
        quad_ptr->width = width;
        quad_ptr->height = height;
        luaL_getmetatable(L, "Quad");
        lua_setmetatable(L, -2);

        return 1;
    }

    static int quad_getWidth(lua_State *L)
    {

        Rectangle *quad_ptr = (Rectangle *)luaL_checkudata(L, 1, "Quad");
        if (quad_ptr == nullptr)
        {

            return luaL_error(L, "Invalid Quad object");
        }

        Rectangle quad = (*(quad_ptr));
        float v = quad.width;
        lua_pushnumber(L, v);
        return 1;
    }

    static int quad_getHeight(lua_State *L)
    {
        Rectangle *quad_ptr = (Rectangle *)luaL_checkudata(L, 1, "Quad");
        if (quad_ptr == nullptr)
        {

            return luaL_error(L, "Invalid Quad object");
        }

        Rectangle quad = (*(quad_ptr));
        float v = quad.height;
        lua_pushnumber(L, v);
        return 1;
    }

    static int gc_quad(lua_State *L)
    {
        Rectangle *quad_ptr = (Rectangle *)luaL_checkudata(L, 1, "Quad");
        if (quad_ptr == nullptr)
        {
            return luaL_error(L, "Invalid Quad object");
        }
        Log(LOG_INFO, "Quad deleted");
        quad_ptr = nullptr;
        return 0;
    }

    int registerQuad(lua_State *L)
    {
        luaL_newmetatable(L, "Quad");
        lua_pushcfunction(L, gc_quad);
        lua_setfield(L, -2, "__gc");
        lua_newtable(L);
        lua_pushcfunction(L, quad_getWidth);
        lua_setfield(L, -2, "getWidth");
        lua_pushcfunction(L, quad_getHeight);
        lua_setfield(L, -2, "getHeight");
        lua_setfield(L, -2, "__index");
        lua_pop(L, 1);
        return 0;
    }

}

namespace nFont
{
    static int newFont(lua_State *L)
    {
        // Recupere os argumentos: filename
        const char *filename = luaL_checkstring(L, 1);
        if (FileExists(filename) == false)
        {
            lua_pushnil(L);
            luaL_error(L, "File not found: %s", filename);
            return 1;
        }
        Font *userdata = (Font *)lua_newuserdata(L, sizeof(Font));
        *userdata = LoadFont(filename);
        luaL_getmetatable(L, "Font");
        lua_setmetatable(L, -2);
        return 1;
    }
    static int font_getWidth(lua_State *L)
    {
        Font *font_ptr = (Font *)luaL_checkudata(L, 1, "Font");
        if (font_ptr == nullptr)
        {
            return luaL_error(L, "Invalid Font object");
        }
        Font font = (*font_ptr);

        const char *text = luaL_checkstring(L, 2);
        float spacing = 1;

        if (lua_gettop(L) == 3)
        {
            spacing = luaL_checknumber(L, 3);
        }
        Vector2 s = MeasureTextEx(font, text, font.baseSize, spacing);
        lua_pushnumber(L, s.x);
        return 1;
    }

    static int font_gc(lua_State *L)
    {
        Font *font_ptr = (Font *)luaL_checkudata(L, 1, "Font");
        if (font_ptr == nullptr)
        {
            return luaL_error(L, "Invalid Font object");
        }
        // Log(LOG_INFO, "Font free");
        Font font = (*font_ptr);
        if (font.texture.id != 0)
        {
            UnloadFont(font);
        }

        return 0;
    }

    int registerFont(lua_State *L)
    {
        luaL_newmetatable(L, "Font");

        lua_pushcfunction(L, font_gc);
        lua_setfield(L, -2, "__gc");

        lua_newtable(L);
        lua_pushcfunction(L, font_getWidth);
        lua_setfield(L, -2, "getWidth");

        lua_setfield(L, -2, "__index");
        lua_pop(L, 1);
        return 0;
    }

}

namespace nImage
{

    static int newGraph(lua_State *L)
    {

        const char *filename = luaL_checkstring(L, 1);
        if (FileExists(filename) == false)
        {
            lua_pushinteger(L, 0);
            luaL_error(L, "File not found: %s", filename);
            return 1;
        }

        int id = _assets.graphics.size();
        if (lua_gettop(L) == 2)
            id = luaL_checknumber(L, 2);
        bool re = _assets.addGraph(filename, id);
        lua_pushinteger(L, id);
        return 1;
    }

    static int newClipGraph(lua_State *L)
    {

        const char *filename = luaL_checkstring(L, 1);
        if (FileExists(filename) == false)
        {
            lua_pushinteger(L, 0);
            luaL_error(L, "File not found: %s", filename);
            return 1;
        }

        int id = _assets.graphics.size();

        float x = luaL_checknumber(L, 2);
        float y = luaL_checknumber(L, 3);
        float w = luaL_checknumber(L, 4);
        float h = luaL_checknumber(L, 5);

        if (lua_gettop(L) == 6)
            id = luaL_checknumber(L, 6);

        bool re = _assets.addGraph(filename, id, x, y, w, h);
        lua_pushinteger(L, id);
        return 1;
    }

    static int addGraph(lua_State *L)
    {

        const char *keys = luaL_checkstring(L, 1);

        int id = _assets.graphics.size();

        float x = luaL_checknumber(L, 2);
        float y = luaL_checknumber(L, 3);
        float w = luaL_checknumber(L, 4);
        float h = luaL_checknumber(L, 5);

        if (lua_gettop(L) == 6)
            id = luaL_checknumber(L, 6);

        bool re = _assets.addSubGraph(keys, id, x, y, w, h);
        lua_pushinteger(L, id);
        return 1;
    }
    static int newGridGraph(lua_State *L)
    {

        const char *filename = luaL_checkstring(L, 1);
        if (FileExists(filename) == false)
        {
            lua_pushinteger(L, 0);
            luaL_error(L, "File not found: %s", filename);
            return 1;
        }

        int id = _assets.graphics.size();
        int count = luaL_checknumber(L, 2);
        int rows = luaL_checknumber(L, 3);
        int columns = luaL_checknumber(L, 4);

        if (lua_gettop(L) == 6)
            id = luaL_checknumber(L, 6);

        bool re = _assets.addGrid(filename, id, count, rows, columns);
        lua_pushinteger(L, id);
        return 1;
    }

    static int newImage(lua_State *L)
    {
        // Recupere os argumentos: filename
        const char *filename = luaL_checkstring(L, 1);
        if (FileExists(filename) == false)
        {
            lua_pushnil(L);
            luaL_error(L, "File not found: %s", filename);
            return 1;
        }
        Texture2D *userdata = (Texture2D *)lua_newuserdata(L, sizeof(Texture2D));
        *userdata = LoadTexture(filename);
        luaL_getmetatable(L, "Image");
        lua_setmetatable(L, -2);
        return 1;
    }

    static int image_getWidth(lua_State *L)
    {
        Texture2D *image_ptr = (Texture2D *)luaL_checkudata(L, 1, "Image");
        if (image_ptr == nullptr)
        {
            return luaL_error(L, "Invalid Image object");
        }
        Texture2D image = (*image_ptr);
        float v = image.width;
        lua_pushnumber(L, v);
        return 1;
    }

    static int image_getHeight(lua_State *L)
    {
        Texture2D *image_ptr = (Texture2D *)luaL_checkudata(L, 1, "Image");
        if (image_ptr == nullptr)
        {
            return luaL_error(L, "Invalid Image object");
        }
        Texture2D image = (*image_ptr);
        float v = image.height;

        lua_pushnumber(L, v);
        return 1;
    }
    static int image_gc(lua_State *L)
    {
        Texture2D *image_ptr = (Texture2D *)luaL_checkudata(L, 1, "Image");

        if (image_ptr == nullptr)
        {
            return luaL_error(L, "Invalid Image object");
        }
        Texture2D image = (*image_ptr);
        if (image.id != 0)
        {
            UnloadTexture(image);
        }
        Log(LOG_INFO, "Image free");
        image_ptr = nullptr;
        return 0;
    }

    int registerImage(lua_State *L)
    {
        luaL_newmetatable(L, "Image");
        lua_pushcfunction(L, image_gc);
        lua_setfield(L, -2, "__gc");

        lua_newtable(L);
        lua_pushcfunction(L, image_getWidth);
        lua_setfield(L, -2, "getWidth");
        lua_pushcfunction(L, image_getHeight);
        lua_setfield(L, -2, "getHeight");
        lua_setfield(L, -2, "__index");
        lua_pop(L, 1);
        return 0;
    }

}

namespace nInput
{

    int coreKeyToRaylib(const char *key)
    {
        if (_keys.find(key) == _keys.end())
        {
            Log(LOG_WARNING, "Key not found: %s", key);
            return -1;
        }
        int key_code = _keys[key];
        return key_code;
    }

    static int core_keyboard_check(lua_State *L)
    {
        int result = 0;
        if (lua_gettop(L) == 1)
        {
            const char *key = luaL_checkstring(L, 1);

            int key_code = coreKeyToRaylib(key);

            result = IsKeyDown(key_code);
        }
        else if (lua_gettop(L) == 2)
        {
            const char *key1 = luaL_checkstring(L, 1);
            const char *key2 = luaL_checkstring(L, 2);
            int key_code1 = coreKeyToRaylib(key1);
            int key_code2 = coreKeyToRaylib(key2);
            result = (int)IsKeyDown(key_code1) || IsKeyDown(key_code2);
        }
        else
        {
            luaL_error(L, "Invalid number of arguments, expected 1 or 2");
        }

        lua_pushinteger(L, result);
        return 1;
    }

    static int core_keyboard_keyPressed(lua_State *L)
    {
        lua_pushinteger(L, GetKeyPressed());
        return 1;
    }
    static int core_keyboard_charPressed(lua_State *L)
    {
        lua_pushinteger(L, GetCharPressed());
        return 1;
    }

    static int core_keyboard_down(lua_State *L)
    {

        bool result = false;
        if (lua_gettop(L) == 1)
        {
            const char *key = luaL_checkstring(L, 1);

            int key_code = coreKeyToRaylib(key);

            result = IsKeyDown(key_code);
        }
        else if (lua_gettop(L) == 2)
        {
            const char *key1 = luaL_checkstring(L, 1);
            const char *key2 = luaL_checkstring(L, 2);
            int key_code1 = coreKeyToRaylib(key1);
            int key_code2 = coreKeyToRaylib(key2);
            result = IsKeyDown(key_code1) || IsKeyDown(key_code2);
        }
        else
        {
            luaL_error(L, "Invalid number of arguments, expected 1 or 2");
        }

        lua_pushboolean(L, result);

        return 1;
    }
    static int core_keyboard_press(lua_State *L)
    {

        bool result = false;
        if (lua_gettop(L) == 1)
        {
            const char *key = luaL_checkstring(L, 1);

            int key_code = coreKeyToRaylib(key);

            result = IsKeyPressed(key_code);
        }
        else if (lua_gettop(L) == 2)
        {
            const char *key1 = luaL_checkstring(L, 1);
            const char *key2 = luaL_checkstring(L, 2);
            int key_code1 = coreKeyToRaylib(key1);
            int key_code2 = coreKeyToRaylib(key2);
            result = IsKeyPressed(key_code1) || IsKeyPressed(key_code2);
        }
        else
        {
            luaL_error(L, "Invalid number of arguments, expected 1 or 2");
        }

        lua_pushboolean(L, result);

        return 1;
    }

    static int core_keyboard_release(lua_State *L)
    {

        bool result = false;
        if (lua_gettop(L) == 1)
        {
            const char *key = luaL_checkstring(L, 1);

            int key_code = coreKeyToRaylib(key);

            result = IsKeyReleased(key_code);
        }
        else if (lua_gettop(L) == 2)
        {
            const char *key1 = luaL_checkstring(L, 1);
            const char *key2 = luaL_checkstring(L, 2);
            int key_code1 = coreKeyToRaylib(key1);
            int key_code2 = coreKeyToRaylib(key2);
            result = IsKeyReleased(key_code1) || IsKeyReleased(key_code2);
        }
        else
        {
            luaL_error(L, "Invalid number of arguments, expected 1 or 2");
        }

        lua_pushboolean(L, result);

        return 1;
    }
    static int core_keyboard_up(lua_State *L)
    {

        bool result = false;
        if (lua_gettop(L) == 1)
        {
            const char *key = luaL_checkstring(L, 1);

            int key_code = coreKeyToRaylib(key);

            result = IsKeyReleased(key_code);
        }
        else if (lua_gettop(L) == 2)
        {
            const char *key1 = luaL_checkstring(L, 1);
            const char *key2 = luaL_checkstring(L, 2);
            int key_code1 = coreKeyToRaylib(key1);
            int key_code2 = coreKeyToRaylib(key2);
            result = IsKeyUp(key_code1) || IsKeyUp(key_code2);
        }
        else
        {
            luaL_error(L, "Invalid number of arguments, expected 1 or 2");
        }

        lua_pushboolean(L, result);

        return 1;
    }
    static int core_keyboard_setExitKey(lua_State *L)
    {
        const char *key = luaL_checkstring(L, 1);
        int key_code = coreKeyToRaylib(key);
        SetExitKey(key_code);
        return 0;
    }

    static int luaopen_keyboard(lua_State *L)
    {
        luaL_Reg reg[] = {
            {"check", core_keyboard_check},
            {"down", core_keyboard_down},
            {"press", core_keyboard_press},
            {"release", core_keyboard_release},
            {"up", core_keyboard_up},
            {"keyPressed", core_keyboard_keyPressed},
            {"charPressed", core_keyboard_charPressed},
            {"setExitKey", core_keyboard_setExitKey},
            {0, 0},
            {0, 0},
        };
        luaL_newlib(L, reg);
        return 1;
    }

    static int core_mouse_press(lua_State *L)
    {
        int button = luaL_checkinteger(L, 1);
        lua_pushboolean(L, IsMouseButtonPressed(button));
        return 1;
    }
    static int core_mouse_release(lua_State *L)
    {
        int button = luaL_checkinteger(L, 1);
        lua_pushboolean(L, IsMouseButtonReleased(button));
        return 1;
    }

    static int core_mouse_up(lua_State *L)
    {
        int button = luaL_checkinteger(L, 1);
        lua_pushboolean(L, IsMouseButtonUp(button));
        return 1;
    }

    static int core_mouse_check(lua_State *L)
    {
        int button = luaL_checkinteger(L, 1);
        lua_pushinteger(L, IsMouseButtonDown(button));
        return 1;
    }

    static int core_mouse_getPosition(lua_State *L)
    {
        Vector2 pos = GetMousePosition();
        lua_pushnumber(L, pos.x);
        lua_pushnumber(L, pos.y);
        return 2;
    }
    static int core_mouse_getWorldPosition(lua_State *L)
    {
        Vector2 pos = GetMousePosition();
        lua_pushnumber(L, pos.x / _camera.zoom - _camera.offset.x + _camera.target.x);
        lua_pushnumber(L, pos.y / _camera.zoom - _camera.offset.x + _camera.target.x);
        return 2;
    }
    static int core_mouse_getDelta(lua_State *L)
    {
        Vector2 pos = GetMouseDelta();
        lua_pushnumber(L, pos.x);
        lua_pushnumber(L, pos.y);
        return 2;
    }

    static int core_mouse_getWorldX(lua_State *L)
    {

        lua_pushnumber(L, GetMouseX() / _camera.zoom - _camera.offset.x + _camera.target.x);
        return 1;
    }
    static int core_mouse_getWorldY(lua_State *L)
    {

        lua_pushnumber(L, GetMouseY() / _camera.zoom - _camera.offset.y + _camera.target.y);
        return 1;
    }
    static int core_mouse_getX(lua_State *L)
    {

        lua_pushnumber(L, GetMouseX());
        return 1;
    }
    static int core_mouse_getY(lua_State *L)
    {

        lua_pushnumber(L, GetMouseY());
        return 1;
    }
    static int core_mouse_getWheel(lua_State *L)
    {
        lua_pushnumber(L, GetMouseWheelMove());
        return 1;
    }

    int luaopen_mouse(lua_State *L)
    {
        luaL_Reg reg[] = {
            {"getPosition", core_mouse_getPosition},
            {"getWorldPosition", core_mouse_getWorldPosition},
            {"getWorldX", core_mouse_getWorldX},
            {"getWorldY", core_mouse_getWorldY},
            {"getX", core_mouse_getX},
            {"getY", core_mouse_getY},
            {"getWheel", core_mouse_getWheel},
            {"check", core_mouse_check},
            {"getDelta", core_mouse_getDelta},
            {"press", core_mouse_press},
            {"release", core_mouse_release},
            {"up", core_mouse_up},
            {0, 0},
        };
        luaL_newlib(L, reg);
        return 1;
    }
}

namespace nTimer
{

    static int core_timer_getTime(lua_State *L)
    {
        lua_pushnumber(L, GetTime());
        return 1;
    }
    static int l_timer_getDelta(lua_State *L)
    {
        lua_pushnumber(L, GetFrameTime());
        return 1;
    }

    static int l_timer_getFPS(lua_State *L)
    {
        lua_pushnumber(L, GetFPS());
        return 1;
    }
    int luaopen_timer(lua_State *L)
    {
        luaL_Reg reg[] = {
            {"getTime", core_timer_getTime},
            {"getDelta", l_timer_getDelta},
            {"getFPS", l_timer_getFPS},

            {0, 0},
        };
        luaL_newlib(L, reg);
        return 1;
    }

}

namespace nSystem
{
    int l_system_getOS(lua_State *L)
    {
        lua_pushstring(L, "Linux");
        return 1;
    }

    int l_system_getMemUsage(lua_State *L)
    {
        lua_pushnumber(L, lua_gc(L, LUA_GCCOUNT, 0) / 1024);
        return 1;
    }

    int luaopen_system(lua_State *L)
    {
        luaL_Reg reg[] = {
            {"getOS", l_system_getOS},
            {"getMemUsage", l_system_getMemUsage},
            {0, 0},
        };
        luaL_newlib(L, reg);
        return 1;
    }
}
namespace nGUI
{
    static int l_gui_disable(lua_State *L)
    {

        GuiDisable();
        return 0;
    }

    static int l_gui_enable(lua_State *L)
    {
        GuiEnable();
        return 0;
    }

    static int l_gui_lock(lua_State *L)
    {

        GuiLock();
        return 0;
    }

    static int l_gui_unlock(lua_State *L)
    {
        GuiUnlock();
        return 0;
    }
    static int l_gui_setState(lua_State *L)
    {
        int state = luaL_checkinteger(L, 1);
        GuiSetState(state);
        return 0;
    }

    static int l_gui_getState(lua_State *L)
    {
        lua_pushinteger(L, GuiGetState());
        return 1;
    }

    static int l_gui_setStyle(lua_State *L)
    {
        int control = luaL_checkinteger(L, 1);
        int property = luaL_checkinteger(L, 2);
        int value = luaL_checkinteger(L, 3);
        GuiSetStyle(control, property, value);
        return 0;
    }

    static int l_gui_getStyle(lua_State *L)
    {
        int control = luaL_checkinteger(L, 1);
        int property = luaL_checkinteger(L, 2);
        lua_pushinteger(L, GuiGetStyle(control, property));
        return 1;
    }

  /*
RAYGUIAPI bool GuiIsLocked(void);                               // Check if gui is locked (global state)
RAYGUIAPI void GuiFade(float alpha);                            // Set gui controls alpha (global state), alpha goes from 0.0f to 1.0f
// Font set/get functions
RAYGUIAPI void GuiSetFont(Font font);                           // Set gui custom font (global state)
  */

    static int l_gui_button(lua_State *L)
    {
        const char *text = luaL_checkstring(L, 1);
        int x = luaL_checkinteger(L, 2);
        int y = luaL_checkinteger(L, 3);
        int width = luaL_checkinteger(L, 4);
        int height = luaL_checkinteger(L, 5);
        int result = GuiButton(Rect(x, y, width, height), text);
        lua_pushboolean(L, result);
        return 1;
    }

    static int l_gui_label(lua_State *L)
    {
        const char *text = luaL_checkstring(L, 1);
        int x = luaL_checkinteger(L, 2);
        int y = luaL_checkinteger(L, 3);
        int width = luaL_checkinteger(L, 4);
        int height = luaL_checkinteger(L, 5);
        GuiLabel(Rect(x, y, width, height), text);
        return 0;
    }

    static int l_gui_labelButton(lua_State *L)
    {
        const char *text = luaL_checkstring(L, 1);
        int x = luaL_checkinteger(L, 2);
        int y = luaL_checkinteger(L, 3);
        int width = luaL_checkinteger(L, 4);
        int height = luaL_checkinteger(L, 5);
        int result = GuiLabelButton(Rect(x, y, width, height), text);
        lua_pushboolean(L, result);
        return 1;
    }

    static int l_gui_toggle(lua_State *L)
    {
        const char *text = luaL_checkstring(L, 1);
        int x = luaL_checkinteger(L, 2);
        int y = luaL_checkinteger(L, 3);
        int width = luaL_checkinteger(L, 4);
        int height = luaL_checkinteger(L, 5);
        bool active = lua_toboolean(L, 6);
        int result = GuiToggle(Rect(x, y, width, height), text, active);
        lua_pushboolean(L, result);
        return 1;
    }

    static int l_gui_toggleGroup(lua_State *L)
    {
        const char *text = luaL_checkstring(L, 1);
        int x = luaL_checkinteger(L, 2);
        int y = luaL_checkinteger(L, 3);
        int width = luaL_checkinteger(L, 4);
        int height = luaL_checkinteger(L, 5);
        int active = luaL_checkinteger(L, 6);
        int result = GuiToggleGroup(Rect(x, y, width, height), text, active);
        lua_pushinteger(L, result);
        return 1;
    }

    static int l_gui_checkBox(lua_State *L)
    {
        const char *text = luaL_checkstring(L, 1);
        int x = luaL_checkinteger(L, 2);
        int y = luaL_checkinteger(L, 3);
        int width = luaL_checkinteger(L, 4);
        int height = luaL_checkinteger(L, 5);
        bool checked = lua_toboolean(L, 6);
        bool result = GuiCheckBox(Rect(x, y, width, height), text, checked);
        lua_pushboolean(L, result);
        return 1;
    }

    static int l_gui_comboBox(lua_State *L)
    {
        const char *text = luaL_checkstring(L, 1);
        int x = luaL_checkinteger(L, 2);
        int y = luaL_checkinteger(L, 3);
        int width = luaL_checkinteger(L, 4);
        int height = luaL_checkinteger(L, 5);
        int active = luaL_checkinteger(L, 6);
        int result = GuiComboBox(Rect(x, y, width, height), text, active);
        lua_pushinteger(L, result);
        return 1;
    }

    static int l_gui_dropDownBox(lua_State *L)
    {
        const char *text = luaL_checkstring(L, 1);
        int x = luaL_checkinteger(L, 2);
        int y = luaL_checkinteger(L, 3);
        int width = luaL_checkinteger(L, 4);
        int height = luaL_checkinteger(L, 5);
        bool editMode = lua_toboolean(L, 6);
        int active;
        bool result = GuiDropdownBox(Rect(x, y, width, height), text, &active, editMode);
        lua_pushboolean(L, result);
        lua_pushinteger(L, active);
        return 2;
    }

    static int l_gui_spinner(lua_State *L)
    {
        const char *text = luaL_checkstring(L, 1);
        int x = luaL_checkinteger(L, 2);
        int y = luaL_checkinteger(L, 3);
        int width = luaL_checkinteger(L, 4);
        int height = luaL_checkinteger(L, 5);
        int minValue = luaL_checkinteger(L, 6);
        int maxValue = luaL_checkinteger(L, 7);
        bool editMode = lua_toboolean(L, 8);

        int value;
        bool result = GuiSpinner(Rect(x, y, width, height), text, &value, minValue, maxValue, editMode);
        lua_pushboolean(L, result);
        lua_pushinteger(L, value);
        return 2;
    }

    static int l_gui_valueBox(lua_State *L)
    {
        const char *text = luaL_checkstring(L, 1);
        int x = luaL_checkinteger(L, 2);
        int y = luaL_checkinteger(L, 3);
        int width = luaL_checkinteger(L, 4);
        int height = luaL_checkinteger(L, 5);

        int minValue = luaL_checkinteger(L, 6);
        int maxValue = luaL_checkinteger(L, 7);
        bool editMode = lua_toboolean(L, 8);

        int value;

        bool result = GuiValueBox(Rect(x, y, width, height), text, &value, minValue, maxValue, editMode);

        lua_pushboolean(L, result);
        lua_pushinteger(L, value);
        return 2;
    }

    static int l_gui_textBox(lua_State *L)
    {
        char *text = (char *)luaL_checkstring(L, 1);
        int textSize = luaL_checkinteger(L, 2);
        int x = luaL_checkinteger(L, 3);
        int y = luaL_checkinteger(L, 4);
        int width = luaL_checkinteger(L, 5);
        int height = luaL_checkinteger(L, 6);
        bool editMode = lua_toboolean(L, 7);
        bool result = GuiTextBox(Rect(x, y, width, height), text, textSize, editMode);
        lua_pushboolean(L, result);
        return 1;
    }

    static int l_gui_slider(lua_State *L)
    {
        const char *textLeft = luaL_checkstring(L, 1);
        const char *textRight = luaL_checkstring(L, 2);
        int x = luaL_checkinteger(L, 3);
        int y = luaL_checkinteger(L, 4);
        int width = luaL_checkinteger(L, 5);
        int height = luaL_checkinteger(L, 6);
        float value = luaL_checknumber(L, 7);
        float min = luaL_checknumber(L, 8);
        float max = luaL_checknumber(L, 9);
        float result = GuiSlider(Rect(x, y, width, height), textLeft, textRight, value, min, max);
        lua_pushnumber(L, result);
        return 1;
    }

    static int l_gui_slider_bar(lua_State *L)
    {
        const char *textLeft = luaL_checkstring(L, 1);
        const char *textRight = luaL_checkstring(L, 2);
        int x = luaL_checkinteger(L, 3);
        int y = luaL_checkinteger(L, 4);
        int width = luaL_checkinteger(L, 5);
        int height = luaL_checkinteger(L, 6);
        float value = luaL_checknumber(L, 7);
        float min = luaL_checknumber(L, 8);
        float max = luaL_checknumber(L, 9);
        float result = GuiSliderBar(Rect(x, y, width, height), textLeft, textRight, value, min, max);
        lua_pushnumber(L, result);
        return 1;
    }
    static int l_gui_progress_bar(lua_State *L)
    {
        const char *textLeft = luaL_checkstring(L, 1);
        const char *textRight = luaL_checkstring(L, 2);
        int x = luaL_checkinteger(L, 3);
        int y = luaL_checkinteger(L, 4);
        int width = luaL_checkinteger(L, 5);
        int height = luaL_checkinteger(L, 6);
        float value = luaL_checknumber(L, 7);
        float min = luaL_checknumber(L, 8);
        float max = luaL_checknumber(L, 9);
        float result = GuiProgressBar(Rect(x, y, width, height), textLeft, textRight, value, min, max);
        lua_pushnumber(L, result);
        return 1;
    }
    static int l_gui_status_bar(lua_State *L)
    {
        const char *text = luaL_checkstring(L, 1);
        int x = luaL_checkinteger(L, 2);
        int y = luaL_checkinteger(L, 3);
        int width = luaL_checkinteger(L, 4);
        int height = luaL_checkinteger(L, 5);
        GuiStatusBar(Rect(x, y, width, height), text);
        return 0;
    }

    static int l_gui_rect(lua_State *L)
    {
        const char *text = luaL_checkstring(L, 1);
        int x = luaL_checkinteger(L, 2);
        int y = luaL_checkinteger(L, 3);
        int width = luaL_checkinteger(L, 4);
        int height = luaL_checkinteger(L, 5);
        GuiDummyRec(Rect(x, y, width, height), text);
        return 0;
    }

    static int l_gui_grid(lua_State *L)
    {
        const char *text = luaL_checkstring(L, 1);
        int x = luaL_checkinteger(L, 2);
        int y = luaL_checkinteger(L, 3);
        int width = luaL_checkinteger(L, 4);
        int height = luaL_checkinteger(L, 5);
        float spacing = luaL_checknumber(L, 6);
        int subdivs = luaL_checkinteger(L, 7);
        Vector2 result = GuiGrid(Rect(x, y, width, height), text, spacing, subdivs);
        lua_pushnumber(L, result.x);
        lua_pushnumber(L, result.y);
        return 2;
    }


/*
// Advance controls set
RAYGUIAPI int GuiListView(Rectangle bounds, const char *text, int *scrollIndex, int active);            // List View control, returns selected list item index
RAYGUIAPI int GuiListViewEx(Rectangle bounds, const char **text, int count, int *focus, int *scrollIndex, int active); // List View with extended parameters
RAYGUIAPI int GuiMessageBox(Rectangle bounds, const char *title, const char *message, const char *buttons); // Message Box control, displays a message
RAYGUIAPI int GuiTextInputBox(Rectangle bounds, const char *title, const char *message, const char *buttons, char *text, int textMaxSize, int *secretViewActive); // Text Input Box control, ask for text, supports secret
RAYGUIAPI Color GuiColorPicker(Rectangle bounds, const char *text, Color color);                        // Color Picker control (multiple color controls)
RAYGUIAPI Color GuiColorPanel(Rectangle bounds, const char *text, Color color);                         // Color Panel control
RAYGUIAPI float GuiColorBarAlpha(Rectangle bounds, const char *text, float alpha);                      // Color Bar Alpha control
RAYGUIAPI float GuiColorBarHue(Rectangle bounds, const char *text, float value);                        // Color Bar Hue control

// Styles loading functions
RAYGUIAPI void GuiLoadStyle(const char *fileName);              // Load style file over global style variable (.rgs)
RAYGUIAPI void GuiLoadStyleDefault(void);                       // Load style default over global style

// Tooltips management functions
RAYGUIAPI void GuiEnableTooltip(void);                          // Enable gui tooltips (global state)
RAYGUIAPI void GuiDisableTooltip(void);                         // Disable gui tooltips (global state)
RAYGUIAPI void GuiSetTooltip(const char *tooltip);              // Set tooltip string
*/
    int luaopen_gui(lua_State *L)
    {

        luaL_Reg reg[] = {
            {"disable", l_gui_disable},
            {"enable", l_gui_enable},

            {"button", l_gui_button},
            {"label", l_gui_label},
            {"labelButton", l_gui_labelButton},
            {"toggle", l_gui_toggle},
            {"toggleGroup", l_gui_toggleGroup},
            {"checkBox", l_gui_checkBox},
            {"comboBox", l_gui_comboBox},
            {"dropBox", l_gui_dropDownBox},
            {"valueBox", l_gui_valueBox},
            {"textBox", l_gui_textBox},
            {"spinner", l_gui_spinner},
            {"slider", l_gui_slider},
            {"slider_bar", l_gui_slider_bar},
            {"progress_bar", l_gui_progress_bar},
            {"status_bar", l_gui_status_bar},
            {"dummy_rec", l_gui_rect},
            {"grid", l_gui_grid},

            {0, 0},
        };
        luaL_newlib(L, reg);
        return 1;
    }

}
namespace nFileSystem
{

    static int l_filesystem_exists(lua_State *L)
    {
        const char *path = luaL_checkstring(L, 1);
        lua_pushboolean(L, FileExists(path));
        return 1;
    }

    static int l_filesystem_isFile(lua_State *L)
    {
        const char *path = luaL_checkstring(L, 1);
        lua_pushboolean(L, FileExists(path));
        return 1;
    }

    static int l_filesystem_isDirectory(lua_State *L)
    {
        const char *path = luaL_checkstring(L, 1);
        lua_pushboolean(L, DirectoryExists(path));
        return 1;
    }

    static int l_filesystem_read(lua_State *L)
    {
        const char *path = luaL_checkstring(L, 1);
        char *buffer = LoadFileText(path);
        if (buffer == NULL)
        {
            lua_pushnil(L);
            lua_pushstring(L, "File not found");
            return 2;
        }
        lua_pushstring(L, buffer);
        free(buffer);
        return 1;
    }

    static int l_filesystem_load(lua_State *L)
    {
        const char *path = luaL_checkstring(L, 1);
        char *buffer = LoadFileText(path);
        if (buffer == NULL)
        {
            lua_pushstring(L, "File not found");
            return 1;
        }
        if (luaL_loadstring(L, buffer) != LUA_OK)
        {

            free(buffer);
            lua_pushstring(L, "Error loading file");
            return 1;
        }
        free(buffer);
        int status = lua_pcall(L, 0, LUA_MULTRET, 0);
        if (status != LUA_OK)
        {
            const char *error_message = lua_tostring(L, -1);
            Log(LOG_ERROR, "%s\n", error_message);

            lua_pushstring(L, error_message);

            return 1;
        }

        lua_pushvalue(L, -1);

        return 1;
    }

    static int l_filesystem_getInfo(lua_State *L)
    {
        const char *path = luaL_checkstring(L, 1);
        lua_newtable(L);
        lua_pushstring(L, "size");
        lua_pushnumber(L, GetFileLength(path));
        lua_settable(L, -3);
        lua_pushstring(L, "modtime");
        lua_pushnumber(L, GetFileModTime(path));
        lua_settable(L, -3);
        return 1;
    }

    static int l_filesystem_getWorkingDirectory(lua_State *L)
    {
        lua_pushstring(L, GetWorkingDirectory());
        return 1;
    }

    static int l_filesystem_getfilePath(lua_State *L)
    {
        const char *path = luaL_checkstring(L, 1);
        lua_pushstring(L, GetDirectoryPath(path));
        return 1;
    }

    int luaopen_filesystem(lua_State *L)
    {
        luaL_Reg reg[] = {
            {"exists", l_filesystem_exists},
            {"isFile", l_filesystem_isFile},
            {"getInfo", l_filesystem_getInfo},
            {"isDirectory", l_filesystem_isDirectory},
            {"getWorkingDirectory", l_filesystem_getWorkingDirectory},
            {"getPath", l_filesystem_getfilePath},
            {"read", l_filesystem_read},
            {"load", l_filesystem_load},

            {0, 0},
        };
        luaL_newlib(L, reg);
        return 1;
    }

}

namespace nSound
{

    static int newSound(lua_State *L)
    {
        const char *filename = luaL_checkstring(L, 1);
        if (FileExists(filename) == false)
        {
            lua_pushnil(L);
            luaL_error(L, "[newSound] File not found: %s", filename);
            return 1;
        }
        Sound *userdata = (Sound *)lua_newuserdata(L, sizeof(Sound));
        *userdata = LoadSound(filename);
        luaL_getmetatable(L, "Sound");
        lua_setmetatable(L, -2);
        return 1;
    }
    static int sound_gc(lua_State *L)
    {
        Sound *sound_ptr = (Sound *)luaL_checkudata(L, 1, "Sound");

        if (sound_ptr == nullptr)
        {
            return luaL_error(L, "[free] Invalid Sound object");
        }
        Sound sound = (*sound_ptr);
        if (sound.stream.buffer != nullptr)
        {
            UnloadSound(sound);
        }

        return 0;
    }

    static int sound_is_playing(lua_State *L)
    {
        Sound *sound_ptr = (Sound *)luaL_checkudata(L, 1, "Sound");

        if (sound_ptr == nullptr)
        {
            return luaL_error(L, "[isPlaying] Invalid Sound object");
        }
        Sound sound = (*sound_ptr);
        if (sound.stream.buffer != nullptr)
        {
            lua_pushboolean(L, IsSoundPlaying(sound));
        }
        else
        {
            lua_pushboolean(L, false);
        }
        return 1;
    }

    static int core_audio_play(lua_State *L)
    {
        Sound *sound_ptr = (Sound *)luaL_checkudata(L, 1, "Sound");

        if (sound_ptr == nullptr)
        {
            return luaL_error(L, "[play] Invalid Sound object");
        }
        Sound sound = (*sound_ptr);
        if (sound.stream.buffer != nullptr)
        {
            PlaySound(sound);
        }

        return 0;
    }

    static int core_audio_pause(lua_State *L)
    {
        Sound *sound_ptr = (Sound *)luaL_checkudata(L, 1, "Sound");

        if (sound_ptr == nullptr)
        {
            return luaL_error(L, "[pause] Invalid Sound object");
        }
        Sound sound = (*sound_ptr);
        if (sound.stream.buffer != nullptr)
        {
            PlaySound(sound);
        }

        return 0;
    }

    static int core_audio_stop(lua_State *L)
    {
        Sound *sound_ptr = (Sound *)luaL_checkudata(L, 1, "Sound");

        if (sound_ptr == nullptr)
        {
            return luaL_error(L, "[stop] Invalid Sound object");
        }
        Sound sound = (*sound_ptr);
        if (sound.stream.buffer != nullptr)
        {
            PlaySound(sound);
        }

        return 0;
    }

    static int core_audio_resume(lua_State *L)
    {
        Sound *sound_ptr = (Sound *)luaL_checkudata(L, 1, "Sound");

        if (sound_ptr == nullptr)
        {
            return luaL_error(L, "[resume] Invalid Sound object");
        }
        Sound sound = (*sound_ptr);
        if (sound.stream.buffer != nullptr)
        {
            PlaySound(sound);
        }

        return 0;
    }
    static int core_audio_setMasterVolume(lua_State *L)
    {
        float volume = luaL_checknumber(L, 1);
        SetMasterVolume(volume);
        return 0;
    }

    static int core_audio_isPlay(lua_State *L)
    {
        Sound *sound_ptr = (Sound *)luaL_checkudata(L, 1, "Sound");
        if (sound_ptr == nullptr)
        {
            return luaL_error(L, "[isPlaying] Invalid Sound object");
        }
        Sound sound = (*sound_ptr);
        bool b = false;
        if (sound.stream.buffer != nullptr)
        {
            b = IsSoundPlaying(sound);
        }
        lua_pushboolean(L, b);

        return 1;
    }

    static int core_audio_setVolume(lua_State *L)
    {
        Sound *sound_ptr = (Sound *)luaL_checkudata(L, 1, "Sound");
        float volume = luaL_checknumber(L, 2);
        if (sound_ptr == nullptr)
        {
            return luaL_error(L, "[setVolume] Invalid Sound object");
        }
        Sound sound = (*sound_ptr);
        if (sound.stream.buffer != nullptr)
            SetSoundVolume(sound, volume);
        return 0;
    }

    static int core_audio_setPitch(lua_State *L)
    {
        Sound *sound_ptr = (Sound *)luaL_checkudata(L, 1, "Sound");
        float pitch = luaL_checknumber(L, 2);
        if (sound_ptr == nullptr)
        {
            return luaL_error(L, "[setPitch] Invalid Sound object");
        }
        Sound sound = (*sound_ptr);
        if (sound.stream.buffer != nullptr)
            SetSoundPitch(sound, pitch);
        return 0;
    }

    static int core_audio_setPan(lua_State *L)
    {
        Sound *sound_ptr = (Sound *)luaL_checkudata(L, 1, "Sound");
        float pan = luaL_checknumber(L, 2);
        if (sound_ptr == nullptr)
        {
            return luaL_error(L, "[setPan] Invalid Sound object");
        }
        Sound sound = (*sound_ptr);
        if (sound.stream.buffer != nullptr)
            SetSoundPan(sound, pan);
        return 0;
    }

    int registerSound(lua_State *L)
    {

        luaL_newmetatable(L, "Sound");
        lua_pushcfunction(L, sound_gc);
        lua_setfield(L, -2, "__gc");

        lua_newtable(L);
        lua_pushcfunction(L, sound_is_playing);
        lua_setfield(L, -2, "isPlaying");
        lua_pushcfunction(L, core_audio_play);
        lua_setfield(L, -2, "play");
        lua_pushcfunction(L, core_audio_pause);
        lua_setfield(L, -2, "pause");
        lua_pushcfunction(L, core_audio_stop);
        lua_setfield(L, -2, "stop");
        lua_pushcfunction(L, core_audio_resume);
        lua_setfield(L, -2, "resume");
        lua_pushcfunction(L, core_audio_setVolume);
        lua_setfield(L, -2, "setVolume");
        lua_pushcfunction(L, core_audio_setPitch);
        lua_setfield(L, -2, "setPitch");
        lua_pushcfunction(L, core_audio_setPan);
        lua_setfield(L, -2, "setPan");

        lua_setfield(L, -2, "__index");
        lua_pop(L, 1);

        return 0;
    }

    static int music_is_ready(lua_State *L)
    {
        Music *music_ptr = (Music *)luaL_checkudata(L, 1, "Music");
        if (music_ptr == nullptr)
        {
            return luaL_error(L, "[isReady] Invalid Music object");
        }
        Music music = (*music_ptr);
        bool b = true; // IsMusicReady(music);
        lua_pushboolean(L, b);
        return 1;
    }

    static int music_is_playing(lua_State *L)
    {
        Music *music_ptr = (Music *)luaL_checkudata(L, 1, "Music");
        if (music_ptr == nullptr)
        {
            return luaL_error(L, "[isPlaying] Invalid Music object");
        }
        Music music = (*music_ptr);
        bool b = IsMusicStreamPlaying(music);
        lua_pushboolean(L, b);
        return 1;
    }

    static int music_play(lua_State *L)
    {
        Music *music_ptr = (Music *)luaL_checkudata(L, 1, "Music");
        if (music_ptr == nullptr)
        {
            return luaL_error(L, "[play] Invalid Music object");
        }
        Music music = (*music_ptr);
        PlayMusicStream(music);
        return 0;
    }

    static int music_pause(lua_State *L)
    {
        Music *music_ptr = (Music *)luaL_checkudata(L, 1, "Music");
        if (music_ptr == nullptr)
        {
            return luaL_error(L, "[pause] Invalid Music object");
        }
        Music music = (*music_ptr);
        PauseMusicStream(music);
        return 0;
    }

    static int music_stop(lua_State *L)
    {
        Music *music_ptr = (Music *)luaL_checkudata(L, 1, "Music");
        if (music_ptr == nullptr)
        {
            return luaL_error(L, "[stop] Invalid Music object");
        }
        Music music = (*music_ptr);
        StopMusicStream(music);
        return 0;
    }

    static int music_resume(lua_State *L)
    {
        Music *music_ptr = (Music *)luaL_checkudata(L, 1, "Music");
        if (music_ptr == nullptr)
        {
            return luaL_error(L, "[resume] Invalid Music object");
        }
        Music music = (*music_ptr);
        ResumeMusicStream(music);
        return 0;
    }

    static int music_seek(lua_State *L)
    {
        Music *music_ptr = (Music *)luaL_checkudata(L, 1, "Music");
        float position = luaL_checknumber(L, 2);
        if (music_ptr == nullptr)
        {
            return luaL_error(L, "[seek] Invalid Music object");
        }
        Music music = (*music_ptr);
        SeekMusicStream(music, position);
        return 0;
    }

    static int music_set_volume(lua_State *L)
    {
        Music *music_ptr = (Music *)luaL_checkudata(L, 1, "Music");
        float volume = luaL_checknumber(L, 2);
        if (music_ptr == nullptr)
        {
            return luaL_error(L, "[setVolume] Invalid Music object");
        }
        Music music = (*music_ptr);
        SetMusicVolume(music, volume);
        return 0;
    }

    static int music_set_pitch(lua_State *L)
    {
        Music *music_ptr = (Music *)luaL_checkudata(L, 1, "Music");
        float pitch = luaL_checknumber(L, 2);
        if (music_ptr == nullptr)
        {
            return luaL_error(L, "[setPitch] Invalid Music object");
        }
        Music music = (*music_ptr);
        SetMusicPitch(music, pitch);
        return 0;
    }

    static int music_set_pan(lua_State *L)
    {
        Music *music_ptr = (Music *)luaL_checkudata(L, 1, "Music");
        float pan = luaL_checknumber(L, 2);
        if (music_ptr == nullptr)
        {
            return luaL_error(L, "[setPan] Invalid Music object");
        }
        Music music = (*music_ptr);
        SetMusicPan(music, pan);
        return 0;
    }

    static int music_get_time_length(lua_State *L)
    {
        Music *music_ptr = (Music *)luaL_checkudata(L, 1, "Music");
        if (music_ptr == nullptr)
        {
            return luaL_error(L, "[getTimeLength] Invalid Music object");
        }
        Music music = (*music_ptr);
        float time = GetMusicTimeLength(music);
        lua_pushnumber(L, time);
        return 1;
    }

    static int music_get_time_played(lua_State *L)
    {
        Music *music_ptr = (Music *)luaL_checkudata(L, 1, "Music");
        if (music_ptr == nullptr)
        {
            return luaL_error(L, "[getTimePlayed] Invalid Music object");
        }
        Music music = (*music_ptr);
        float time = GetMusicTimePlayed(music);
        lua_pushnumber(L, time);
        return 1;
    }

    static int newMusic(lua_State *L)
    {
        const char *filename = luaL_checkstring(L, 1);
        if (FileExists(filename) == false)
        {
            lua_pushnil(L);
            luaL_error(L, "[newMusic] File not found: %s", filename);
            return 1;
        }
        Music *userdata = (Music *)lua_newuserdata(L, sizeof(Music));
        *userdata = LoadMusicStream(filename);
        luaL_getmetatable(L, "Music");
        lua_setmetatable(L, -2);
        return 1;
    }
    static int music_gc(lua_State *L)
    {
        Music *music_ptr = (Music *)luaL_checkudata(L, 1, "Music");
        if (music_ptr == nullptr)
        {
            return luaL_error(L, "[free] Invalid Music object");
        }
        Music music = (*music_ptr);
        if (music.stream.buffer != nullptr)
        {
            UnloadMusicStream(music);
        }

        return 0;
    }

    int registerMusic(lua_State *L)
    {

        luaL_newmetatable(L, "Music");
        lua_pushcfunction(L, music_gc);
        lua_setfield(L, -2, "__gc");

        lua_newtable(L);
        lua_pushcfunction(L, music_is_playing);
        lua_setfield(L, -2, "isPlaying");
        lua_pushcfunction(L, music_play);
        lua_setfield(L, -2, "play");
        lua_pushcfunction(L, music_pause);
        lua_setfield(L, -2, "pause");
        lua_pushcfunction(L, music_stop);
        lua_setfield(L, -2, "stop");
        lua_pushcfunction(L, music_resume);
        lua_setfield(L, -2, "resume");
        lua_pushcfunction(L, music_seek);
        lua_setfield(L, -2, "seek");
        lua_pushcfunction(L, music_set_volume);
        lua_setfield(L, -2, "setVolume");
        lua_pushcfunction(L, music_set_pitch);
        lua_setfield(L, -2, "setPitch");
        lua_pushcfunction(L, music_set_pan);
        lua_setfield(L, -2, "setPan");
        lua_pushcfunction(L, music_get_time_length);
        lua_setfield(L, -2, "getTimeLength");
        lua_pushcfunction(L, music_get_time_played);
        lua_setfield(L, -2, "getTimePlayed");
        lua_setfield(L, -2, "__index");
        lua_pop(L, 1);

        return 0;
    }

    /// sound function

    int luaopen_sound(lua_State *L)
    {
        luaL_Reg reg[] = {
            {"setMasterVolume", core_audio_setMasterVolume},
            {"newSound", newSound},
            {"newMusic", newMusic},
            {0, 0},
        };
        luaL_newlib(L, reg);

        return 1;
    }

}

namespace ncore
{

    int l_core_getVersion(lua_State *L)
    {
        lua_pushstring(L, CORE_VERSION);
        return 1;
    }

    int l_core_setup(lua_State *L)
    {
        screenWidth = luaL_checkinteger(L, 1);
        screenHeight = luaL_checkinteger(L, 2);
        screenFps = luaL_checkinteger(L, 3);
        screenTitle = luaL_checkstring(L, 4);
        screenVsync = lua_toboolean(L, 5);
        screenFullscreen = lua_toboolean(L, 6);
        screenBorderless = lua_toboolean(L, 7);
        screenResizable = lua_toboolean(L, 8);

        return 0;
    }
    int l_core_init(lua_State *L)
    {
        (void)L;
        int flags = 0;
        if (screenVsync)
        {
            flags |= FLAG_VSYNC_HINT;
        }
        if (screenFullscreen)
        {
            flags |= FLAG_FULLSCREEN_MODE;
        }
        if (screenBorderless)
        {
            flags |= FLAG_WINDOW_UNDECORATED;
        }
        if (screenResizable)
        {
            flags |= FLAG_WINDOW_RESIZABLE;
        }
        SetConfigFlags(flags);
        InitWindow(screenWidth, screenHeight, screenTitle.c_str());
        Log(LOG_INFO, "Core (%s) initialized", CORE_VERSION);
        SetTargetFPS(screenFps);
        InitAudioDevice();
        Log(LOG_INFO, "Audio  initialized");
        init_physics();
        screenInit = true;

        return 0;
    }

    static int l_core_close(lua_State *L)
    {
        lua_pushboolean(L, WindowShouldClose());
        return 1;
    }

    static int l_core_log(lua_State *L)
    {
        if (lua_gettop(L) != 2)
        {
            return luaL_error(L, "log: wrong number of arguments (expected 2, got %d)", lua_gettop(L));
        }

        int type = luaL_checkinteger(L, 1);
        const char *msg = luaL_checkstring(L, 2);

        switch (type)
        {
        case 0:
            Log(LOG_INFO, "%s", msg);
            break;
        case 1:
            Log(LOG_WARNING, "%s", msg);
            break;
        case 2:
            Log(LOG_ERROR, "%s", msg);
            break;
        }

        return 0;
    }

    static int l_core_set_window_size(lua_State *L)
    {
        if (lua_gettop(L) != 2)
        {
            return luaL_error(L, "setWindowSize: wrong number of arguments (expected 2, got %d)", lua_gettop(L));
        }

        int width = luaL_checkinteger(L, 1);
        int height = luaL_checkinteger(L, 2);

        SetWindowSize(width, height);

        return 0;
    }

    static int l_core_set_window_opacity(lua_State *L)
    {
        if (lua_gettop(L) != 1)
        {
            return luaL_error(L, "setWindowOpacity: wrong number of arguments (expected 1, got %d)", lua_gettop(L));
        }

        float opacity = luaL_checknumber(L, 1);

        SetWindowOpacity(opacity);

        return 0;
    }

    static int l_core_get_screen_width(lua_State *L)
    {
        lua_pushinteger(L, GetScreenWidth());
        return 1;
    }

    static int l_core_get_screen_height(lua_State *L)
    {
        lua_pushinteger(L, GetScreenHeight());
        return 1;
    }

    static int l_core_get_render_width(lua_State *L)
    {
        lua_pushinteger(L, GetRenderWidth());
        return 1;
    }

    static int l_core_get_render_height(lua_State *L)
    {
        lua_pushinteger(L, GetRenderHeight());
        return 1;
    }

    static int l_core_get_monitor_count(lua_State *L)
    {
        lua_pushinteger(L, GetMonitorCount());
        return 1;
    }

    static int l_core_get_current_monitor(lua_State *L)
    {
        lua_pushinteger(L, GetCurrentMonitor());
        return 1;
    }

    static int l_core_get_monitor_position(lua_State *L)
    {
        if (lua_gettop(L) != 1)
        {
            return luaL_error(L, "getMonitorPosition: wrong number of arguments (expected 1, got %d)", lua_gettop(L));
        }

        int monitor = luaL_checkinteger(L, 1);

        Vector2 position = GetMonitorPosition(monitor);

        lua_pushinteger(L, position.x);
        lua_pushinteger(L, position.y);

        return 2;
    }

    static int l_core_get_monitor_width(lua_State *L)
    {
        if (lua_gettop(L) != 1)
        {
            return luaL_error(L, "getMonitorWidth: wrong number of arguments (expected 1, got %d)", lua_gettop(L));
        }

        int monitor = luaL_checkinteger(L, 1);

        lua_pushinteger(L, GetMonitorWidth(monitor));

        return 1;
    }

    static int l_core_get_monitor_height(lua_State *L)
    {
        if (lua_gettop(L) != 1)
        {
            return luaL_error(L, "getMonitorHeight: wrong number of arguments (expected 1, got %d)", lua_gettop(L));
        }

        int monitor = luaL_checkinteger(L, 1);

        lua_pushinteger(L, GetMonitorHeight(monitor));

        return 1;
    }

    static int l_core_get_monitor_physical_width(lua_State *L)
    {
        if (lua_gettop(L) != 1)
        {
            return luaL_error(L, "getMonitorPhysicalWidth: wrong number of arguments (expected 1, got %d)", lua_gettop(L));
        }

        int monitor = luaL_checkinteger(L, 1);

        lua_pushinteger(L, GetMonitorPhysicalWidth(monitor));

        return 1;
    }

    static int l_core_get_monitor_physical_height(lua_State *L)
    {
        if (lua_gettop(L) != 1)
        {
            return luaL_error(L, "getMonitorPhysicalHeight: wrong number of arguments (expected 1, got %d)", lua_gettop(L));
        }

        int monitor = luaL_checkinteger(L, 1);

        lua_pushinteger(L, GetMonitorPhysicalHeight(monitor));

        return 1;
    }

    static int l_core_get_monitor_refresh_rate(lua_State *L)
    {
        if (lua_gettop(L) != 1)
        {
            return luaL_error(L, "getMonitorRefreshRate: wrong number of arguments (expected 1, got %d)", lua_gettop(L));
        }

        int monitor = luaL_checkinteger(L, 1);

        lua_pushinteger(L, GetMonitorRefreshRate(monitor));

        return 1;
    }

    static int l_core_get_window_position(lua_State *L)
    {
        Vector2 position = GetWindowPosition();

        lua_pushinteger(L, position.x);
        lua_pushinteger(L, position.y);

        return 2;
    }

    static int l_core_set_window_position(lua_State *L)
    {
        if (lua_gettop(L) != 2)
        {
            return luaL_error(L, "setWindowPosition: wrong number of arguments (expected 2, got %d)", lua_gettop(L));
        }

        int x = luaL_checkinteger(L, 1);
        int y = luaL_checkinteger(L, 2);

        SetWindowPosition(x, y);

        return 0;
    }

    static int l_core_set_window_title(lua_State *L)
    {
        if (lua_gettop(L) != 1)
        {
            return luaL_error(L, "setWindowTitle: wrong number of arguments (expected 1, got %d)", lua_gettop(L));
        }

        const char *title = luaL_checkstring(L, 1);

        SetWindowTitle(title);

        return 0;
    }
    static int l_core_set_clipboad(lua_State *L)
    {
        if (lua_gettop(L) != 1)
        {
            return luaL_error(L, "setClipboard: wrong number of arguments (expected 1, got %d)", lua_gettop(L));
        }

        const char *text = luaL_checkstring(L, 1);

        SetClipboardText(text);

        return 0;
    }

    static int l_core_get_clipboad(lua_State *L)
    {
        const char *text = GetClipboardText();

        lua_pushstring(L, text);

        return 1;
    }

    /*



        static int l_core_get_render_width(lua_State *L)
        static int l_core_get_render_height(lua_State *L)
        static int l_core_get_monitor_count(lua_State *L)
        static int l_core_get_current_monitor(lua_State *L)
        static int l_core_get_monitor_position(lua_State *L)
        static int l_core_get_monitor_width(lua_State *L)
        static int l_core_get_monitor_height(lua_State *L)
        static int l_core_get_monitor_physical_width(lua_State *L)
        static int l_core_get_monitor_physical_height(lua_State *L)
        static int l_core_get_window_position(lua_State *L)
        static int l_core_set_window_position(lua_State *L)
        static int l_core_set_window_title(lua_State *L)
        static int l_core_set_window_icon(lua_State *L)
        static int l_core_set_clipboad(lua_State *L)
        static int l_core_get_clipboad(lua_State *L)

    */

    int core_api(lua_State *L)
    {
        luaL_Reg reg[] = {
            {"getVersion", l_core_getVersion},
            {"init", l_core_init},
            {"setup", l_core_setup},
            {"close", l_core_close},
            {"log", l_core_log},
            {"setWindowPosition", l_core_set_window_position},
            {"getsetWindowPosition", l_core_get_window_position},
            {"setWindowSize", l_core_set_window_size},
            {"getWidth", l_core_get_screen_width},
            {"getHeight", l_core_get_screen_height},

            {0, 0},
        };
        luaL_newlib(L, reg);
        return 1;
    }
}

int luaopen_graphics(lua_State *L)
{

    using namespace nGraphics;

    luaL_Reg reg[] = {
        {"getWidth", core_graphics_getWidth},
        {"getHeight", core_graphics_getHeight},
        {"setBackgroundColor", core_graphics_setBackgroundColor},

        {"setColor", core_graphics_setColor},
        {"getFont", core_graphics_getFont},

        {"beginCamera", core_graphics_begin_camera},
        {"endCamera", core_graphics_end_camera},

        {"setCameraOrigin", core_graphics_set_camera_offset},
        {"getCameraOrigin", core_graphics_get_camera_offset},

        {"setCameraPosition", core_graphics_set_camera_target},
        {"getCameraPosition", core_graphics_get_camera_target},

        {"setCameraZoom", core_graphics_set_camera_zoom},
        {"getCameraZoom", core_graphics_get_camera_zoom},

        {"setCameraRotation", core_graphics_set_camera_rotation},
        {"getCameraRotation", core_graphics_get_camera_rotation},

        {"pop", core_graphics_pop},
        {"push", core_graphics_push},
        {"rotate", core_graphics_rotate},
        {"scale", core_graphics_scale},
        {"translate", core_graphics_translate},
        {"shear", core_graphics_shear},

        {"clear", core_graphics_clear},
        {"begin", core_graphics_begin},
        {"present", core_graphics_end},
        {"draw", core_graphics_draw},
        {"drawGraph", core_graphics_draw_graph},
        {"point", core_graphics_point},
        {"line", core_graphics_line},
        {"rectangle", core_graphics_rectangle},
        {"circle", core_graphics_circle},
        {"print", core_graphics_print},
        {"drawFPS", core_graphics_drawFps},
        {"newGraph", nImage::newGraph},
        {"addGraph", nImage::addGraph},
        {"newGridGraph", nImage::newGridGraph},
        {"newClipGraph", nImage::newClipGraph},
        {"newImage", nImage::newImage},
        {"newQuad", nQuad::newQuad},
        {"newFont", nFont::newFont},
        {0, 0},
    };
    luaL_newlib(L, reg);
    return 1;
}
int luaopen_core(lua_State *L)
{
    using namespace ncore;

    core_api(L);

    nSound::registerMusic(L);
    nSound::registerSound(L);
    nImage::registerImage(L);
    nQuad::registerQuad(L);
    nFont::registerFont(L);
    luaclass_physics(L);

    /* Init submodules */
    nSystem::luaopen_system(L);
    lua_setfield(L, -2, "system");

    nFileSystem::luaopen_filesystem(L);
    lua_setfield(L, -2, "filesystem");

    nGUI::luaopen_gui(L);
    lua_setfield(L, -2, "gui");

    luaopen_graphics(L);
    lua_setfield(L, -2, "graphics");

    nTimer::luaopen_timer(L);
    lua_setfield(L, -2, "timer");

    nInput::luaopen_keyboard(L);
    lua_setfield(L, -2, "keyboard");

    nInput::luaopen_mouse(L);
    lua_setfield(L, -2, "mouse");

    nSound::luaopen_sound(L);
    lua_setfield(L, -2, "audio");

    luaopen_physics(L);
    lua_setfield(L, -2, "physics");

    return 1;
}

struct MainScript
{

    std::string path;
    bool isLoad;
    bool panic;
    long timeLoad;
    bool enableLiveReload = {true};
    float lastCheckTime = {0};
    float checkInterval = {5};
    std::unordered_map<std::string, int> luaFunctions;
    lua_State *L;

    MainScript()
    {
        L = nullptr;
        isLoad = false;
        panic = false;
        timeLoad = 0;
    }
    void release()
    {
        collect();
    }

    void init(lua_State *state)
    {
        L = state;
        isLoad = false;
        panic = false;
        timeLoad = 0;
    }
    bool isFunctionRegistered(const std::string &functionName)
    {

        auto it = luaFunctions.find(functionName);
        return it != luaFunctions.end();
    }

    void registerFunction(const char *functionName)
    {
        lua_getglobal(L, functionName);
        if (lua_isfunction(L, -1))
        {
            luaFunctions[functionName] = luaL_ref(L, LUA_REGISTRYINDEX);
        }
        else
        {
            lua_pop(L, 1); // Remover o valor não-função do topo da pilha
        }
    }
    void LuaBind()
    {
        luaFunctions.clear();
        registerFunction("load");
        registerFunction("ready");
        registerFunction("close");
        registerFunction("draw");
        registerFunction("update");
    }
    void watch()
    {
        if (!enableLiveReload)
            return;
        float currentTime = GetTime();
        if (currentTime - lastCheckTime >= checkInterval)
        {
            lastCheckTime = currentTime;

            if (isLoad)
            {
                if (FileExists(path.c_str()))
                {
                    if (timeLoad != GetFileModTime(path.c_str()))
                    {
                        timeLoad = GetFileModTime(path.c_str());
                        Log(LOG_INFO, "Reload %s", path.c_str());
                        Close();
                        collect();
                        Load(path.c_str());
                        Create();
                        collect();
                    }
                }
                else
                {
                    isLoad = false;
                }
            }
        }
    }
    void collect()
    {

        Log(LOG_INFO, "Collecting garbage");
        lua_gc(L, LUA_GCCOLLECT, 0);
    }
    void Load(const char *filename)
    {

        isLoad = false;
        if (FileExists(filename))
        {
            this->path = filename;
            Log(LOG_INFO, "Loading %s", filename);
            isLoad = true;
        }
        else if (FileExists("scripts/main.lua"))
        {
            this->path = "scripts/main.lua";
            isLoad = true;
            Log(LOG_INFO, "Loading scripts/main.lua");
        }
        else if (FileExists("../scripts/main.lua"))
        {
            this->path = "../scripts/main.lua";
            isLoad = true;
            Log(LOG_INFO, "Loading ../scripts/main.lua");
        }
        else if (FileExists("assets/main.lua"))
        {

            this->path = "assets/main.lua";
            isLoad = true;
            Log(LOG_INFO, "Loading assets/main.lua");
        }
        else if (FileExists("../assets/main.lua"))
        {

            this->path = "../assets/main.lua";
            isLoad = true;
            Log(LOG_INFO, "Loading ../assets/main.lua");
        }
        else if (FileExists("assets/scripts/main.lua"))
        {

            this->path = "assets/scripts/main.lua";
            isLoad = true;
            Log(LOG_INFO, "Loading assets/scripts/main.lua");
        }
        else if (FileExists("../assets/scripts/main.lua"))
        {

            this->path = "../assets/scripts/main.lua";
            isLoad = true;
            Log(LOG_INFO, "Loading ../assets/scripts/main.lua");
        }

        if (!isLoad)
        {
            Log(LOG_WARNING, "Failed to load script %s", filename);
            return;
        }

        timeLoad = GetFileModTime(this->path.c_str());
        char *data = LoadFileText(this->path.c_str());
        size_t size = strlen(data);

        if (luaL_loadbufferx(L, data, size, this->path.c_str(), nullptr) != LUA_OK)
        {
            const char *errMsg = lua_tostring(L, -1);
            Log(LOG_ERROR, "Failed to load script %s : %s ", this->path.c_str(), errMsg);
            lua_pop(L, 1);
            free(data);
            panic = true;
            return;
        }

        free(data);

        if (lua_pcall(L, 0, LUA_MULTRET, 0) != LUA_OK)
        {
            const char *errMsg = lua_tostring(L, -1);
            Log(LOG_ERROR, "Failed to execute script %s : %s ", this->path.c_str(), errMsg);
            lua_pop(L, 1);
            panic = true;
            return;
        }
        panic = false;

        LuaBind();
    }

    void Update(float dt)
    {
        if (panic || !isLoad)
        {
            return;
        }

        if (!isFunctionRegistered("update"))
        {
            return;
        }

        int updateRef = luaFunctions["update"];
        lua_rawgeti(L, LUA_REGISTRYINDEX, updateRef);
        lua_pushnumber(L, dt);

        if (lua_pcall(L, 1, 0, 0) != LUA_OK)
        {
            const char *errMsg = lua_tostring(L, -1);
            Log(LOG_ERROR, "Failed to execute script %s [update] : %s ", path.c_str(), errMsg);
            lua_pop(L, 1);
            panic = true;
            return;
        }
    }
    void Render()
    {
        if (panic || !isLoad)
        {
            return;
        }

        if (!isFunctionRegistered("draw"))
        {
            return;
        }

        int updateRef = luaFunctions["draw"];
        lua_rawgeti(L, LUA_REGISTRYINDEX, updateRef);
        if (lua_pcall(L, 0, 0, 0) != LUA_OK)
        {
            const char *errMsg = lua_tostring(L, -1);
            Log(LOG_ERROR, "Failed to execute script %s [draw] : %s ", path.c_str(), errMsg);
            lua_pop(L, 1);
            panic = true;
            return;
        }
    }

    void Create()
    {
        if (panic || !isLoad)
        {
            return;
        }

        if (!isFunctionRegistered("load"))
        {
            return;
        }

        int updateRef = luaFunctions["load"];
        lua_rawgeti(L, LUA_REGISTRYINDEX, updateRef);
        if (lua_pcall(L, 0, 0, 0) != LUA_OK)
        {
            const char *errMsg = lua_tostring(L, -1);
            Log(LOG_ERROR, "Failed to execute script %s [load] : %s ", path.c_str(), errMsg);
            lua_pop(L, 1);
            panic = true;
            return;
        }
    }
    void Ready()
    {
        if (panic || !isLoad)
        {
            return;
        }

        if (!isFunctionRegistered("ready"))
        {
            return;
        }

        int updateRef = luaFunctions["ready"];
        lua_rawgeti(L, LUA_REGISTRYINDEX, updateRef);
        if (lua_pcall(L, 0, 0, 0) != LUA_OK)
        {
            const char *errMsg = lua_tostring(L, -1);
            Log(LOG_ERROR, "Failed to execute script %s [ready] : %s ", path.c_str(), errMsg);
            lua_pop(L, 1);
            panic = true;
            return;
        }
    }
    void Close()
    {
        if (panic || !isLoad)
        {
            return;
        }

        if (!isFunctionRegistered("close"))
        {
            return;
        }

        int updateRef = luaFunctions["close"];
        lua_rawgeti(L, LUA_REGISTRYINDEX, updateRef);
        if (lua_pcall(L, 0, 0, 0) != LUA_OK)
        {
            const char *errMsg = lua_tostring(L, -1);
            Log(LOG_ERROR, "Failed to execute script %s [close] : %s ", path.c_str(), errMsg);
            lua_pop(L, 1);
            panic = true;
            return;
        }
    }
};
MainScript mainScript;

void CreateCore(lua_State *L)
{

    using namespace nQuad;
    using namespace nImage;
    using namespace nInput;

    _color = WHITE;
    _backgroundColor = BLACK;
    _camera.offset.x = 0;
    _camera.offset.y = 0;
    _camera.rotation = 0;
    _camera.zoom = 1.0f;
    _camera.target.x = 0;
    _camera.target.y = 0;

    //  luaL_dostring(L, "package.path = package.path .. ';scripts/?.lua;assets/scripts/?.lua;assets/scripts/utils/?.lua'");

    registerQuad(L);
    registerImage(L);
    luaL_requiref(L, "core", luaopen_core, 1);
    mainScript.init(L);
}

void CloseCore(lua_State *L)
{

    mainScript.Close();
    mainScript.release();
    lua_close(L);
    if (screenInit)
    {
        _assets.clear();
        free_physics();
        CloseAudioDevice();
        Log(LOG_INFO, "Audio closed");

        CloseWindow();
        Log(LOG_INFO, "Core closed");
    }
}

void CoreLoad(int argc, char **argv)
{
    if (!screenInit)
        return;
    const char *filename;
    if (argc > 1)
    {
        filename = argv[1];
        Log(LOG_INFO, "Loading %s", filename);
    }
    else
    {
        filename = "assets/scripts/main.lua";
        Log(LOG_INFO, "Loading assets/scripts/main.lua");
    }
    mainScript.Load(filename);
}

void LoopCore()
{
    if (!screenInit)
        return;

    mainScript.Create();
    mainScript.Ready();
    _keys["a"] = KEY_A;
    _keys["b"] = KEY_B;
    _keys["c"] = KEY_C;
    _keys["d"] = KEY_D;
    _keys["e"] = KEY_E;
    _keys["f"] = KEY_F;
    _keys["g"] = KEY_G;
    _keys["h"] = KEY_H;
    _keys["i"] = KEY_I;
    _keys["j"] = KEY_J;
    _keys["k"] = KEY_K;
    _keys["l"] = KEY_L;
    _keys["m"] = KEY_M;
    _keys["n"] = KEY_N;
    _keys["o"] = KEY_O;
    _keys["p"] = KEY_P;
    _keys["q"] = KEY_Q;
    _keys["r"] = KEY_R;
    _keys["s"] = KEY_S;
    _keys["t"] = KEY_T;
    _keys["u"] = KEY_U;
    _keys["v"] = KEY_V;
    _keys["w"] = KEY_W;
    _keys["x"] = KEY_X;
    _keys["y"] = KEY_Y;
    _keys["z"] = KEY_Z;
    _keys["0"] = KEY_ZERO;
    _keys["1"] = KEY_ONE;
    _keys["2"] = KEY_TWO;
    _keys["3"] = KEY_THREE;
    _keys["4"] = KEY_FOUR;
    _keys["5"] = KEY_FIVE;
    _keys["6"] = KEY_SIX;
    _keys["7"] = KEY_SEVEN;
    _keys["8"] = KEY_EIGHT;
    _keys["9"] = KEY_NINE;
    _keys["space"] = KEY_SPACE;
    _keys["esc"] = KEY_ESCAPE;
    _keys["enter"] = KEY_ENTER;
    _keys["backspace"] = KEY_BACKSPACE;
    _keys["tab"] = KEY_TAB;
    _keys["left"] = KEY_LEFT;
    _keys["right"] = KEY_RIGHT;
    _keys["up"] = KEY_UP;
    _keys["down"] = KEY_DOWN;
    _keys["f1"] = KEY_F1;
    _keys["f2"] = KEY_F2;
    _keys["f3"] = KEY_F3;
    _keys["f4"] = KEY_F4;
    _keys["f5"] = KEY_F5;
    _keys["f6"] = KEY_F6;
    _keys["f7"] = KEY_F7;
    _keys["f8"] = KEY_F8;
    _keys["f9"] = KEY_F9;
    _keys["f10"] = KEY_F10;
    _keys["f11"] = KEY_F11;
    _keys["f12"] = KEY_F12;
    _keys["pause"] = KEY_PAUSE;
    _keys["insert"] = KEY_INSERT;
    _keys["home"] = KEY_HOME;
    _keys["pageup"] = KEY_PAGE_UP;
    _keys["pagedown"] = KEY_PAGE_DOWN;
    _keys["end"] = KEY_END;
    _keys["del"] = KEY_DELETE;
    _keys["capslock"] = KEY_CAPS_LOCK;
    _keys["scrolllock"] = KEY_SCROLL_LOCK;
    _keys["numlock"] = KEY_NUM_LOCK;
    _keys["printscreen"] = KEY_PRINT_SCREEN;
    _keys["rightshift"] = KEY_RIGHT_SHIFT;
    _keys["leftshift"] = KEY_LEFT_SHIFT;
    _keys["rightcontrol"] = KEY_RIGHT_CONTROL;
    _keys["leftcontrol"] = KEY_LEFT_CONTROL;
    _keys["rightalt"] = KEY_RIGHT_ALT;
    _keys["leftalt"] = KEY_LEFT_ALT;
    _keys["rightsuper"] = KEY_RIGHT_SUPER;

    while (!WindowShouldClose())
    {
        mainScript.watch();
        mainScript.Update(GetFrameTime());
        BeginDrawing();
        ClearBackground(_backgroundColor);
        mainScript.Render();
        EndDrawing();
    }
}

#endif