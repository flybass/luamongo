#include <iostream>
#include <client/dbclient.h>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#if !defined(LUA_VERSION_NUM) || (LUA_VERSION_NUM < 501)
#include <compat-5.1.h>
#endif
};

#include "utils.h"
#include "common.h"

using namespace mongo;

extern void push_bsontype_table(lua_State* L, mongo::BSONType bsontype);
void lua_push_value(lua_State *L, const BSONElement &elem);
const char *bson_name(int type);

static int check_array(lua_State *L, int stackpos) {
    int type = lua_type(L, stackpos);
    int maxindex = 0;

    if (type != LUA_TTABLE) {
        return 0;
    }

    int tablelen = lua_objlen(L, stackpos);

    lua_pushnil(L);
    while (lua_next(L, stackpos-1) != 0) {
        if (lua_isnumber(L, -2)) {
            double index = lua_tonumber(L, -2);

            if (index == floor(index)) {
                int indexint = (int)floor(index);

                maxindex = max(indexint, maxindex);

                lua_pop(L, 1);
            } else {
                lua_pop(L, 2);
                return 0;
            }
        } else {
            lua_pop(L, 2);
            return 0;
        }
    }

    if (tablelen != maxindex) {
        return 0;
    }

    return tablelen;
}

static void bson_to_array(lua_State *L, const BSONObj &obj) {
    BSONObjIterator it = BSONObjIterator(obj);

    lua_newtable(L);

    int n = 1;
    while (it.more()) {
        BSONElement elem = it.next();

        lua_push_value(L, elem);
        lua_rawseti(L, -2, n++);
    }
}

static void bson_to_table(lua_State *L, const BSONObj &obj) {
    BSONObjIterator it = BSONObjIterator(obj);

    lua_newtable(L);

    while (it.more()) {
        BSONElement elem = it.next();
        const char *key = elem.fieldName();

        lua_pushstring(L, key);
        lua_push_value(L, elem);
        lua_rawset(L, -3);
    }
}

void lua_push_value(lua_State *L, const BSONElement &elem) {
    int type = elem.type();

    switch(type) {
    case mongo::Undefined:
        lua_pushnil(L);
        break;
    case mongo::NumberInt:
        lua_pushinteger(L, elem.numberInt());
        break;
    case mongo::NumberLong:
    case mongo::NumberDouble:
        lua_pushnumber(L, elem.number());
        break;
    case mongo::Bool:
        lua_pushboolean(L, elem.boolean());
        break;
    case mongo::String:
        lua_pushstring(L, elem.valuestr());
        break;
    case mongo::Array:
        bson_to_array(L, elem.embeddedObject());
        break;
    case mongo::Object:
        bson_to_table(L, elem.embeddedObject());
        break;
    case mongo::Date:
        push_bsontype_table(L, mongo::Date);
        lua_pushnumber(L, elem.date());
        lua_rawseti(L, -2, 1);
        break;
    case mongo::Timestamp:
        push_bsontype_table(L, mongo::Date);
        lua_pushnumber(L, elem.timestampTime());
        lua_rawseti(L, -2, 1);
        break;
    case mongo::Symbol:
        push_bsontype_table(L, mongo::Symbol);
        lua_pushstring(L, elem.valuestr());
        lua_rawseti(L, -2, 1);
        break;
    case mongo::RegEx:
        push_bsontype_table(L, mongo::RegEx);
        lua_pushstring(L, elem.regex());
        lua_rawseti(L, -2, 1);
        lua_pushstring(L, elem.regexFlags());
        lua_rawseti(L, -2, 2);
        break;
    case mongo::jstOID:
        push_bsontype_table(L, mongo::jstOID);
        lua_pushstring(L, elem.__oid().str().c_str());
        lua_rawseti(L, -2, 1);
        break;
    case mongo::jstNULL:
        push_bsontype_table(L, mongo::jstNULL);
        break;
    case mongo::EOO:
        break;
    default:
        luaL_error(L, LUAMONGO_UNSUPPORTED_BSON_TYPE, bson_name(type));
    }
}

static void lua_append_bson(lua_State *L, const char *key, int stackpos, BSONObjBuilder *builder) {
    int type = lua_type(L, stackpos);

    if (type == LUA_TTABLE) {
        int bsontype_found = luaL_getmetafield(L, stackpos, "__bsontype");
        if (!bsontype_found) {
            // not a special bsontype
            // handle as a regular table, iterating keys
            BSONObjBuilder b;

            int arraylen = check_array(L, stackpos);

            if (arraylen) {
                for (int i = 0; i < arraylen; i++) {
                    lua_rawgeti(L, stackpos, i+1);

                    stringstream ss;
                    ss << i;

                    lua_append_bson(L, ss.str().c_str(), -1, &b);
                    lua_pop(L, 1);
                }

                builder->appendArray(key, b.obj());
            } else {
                lua_pushnil(L);
                while (lua_next(L, stackpos-1) != 0) {
                    if (lua_isnumber(L, -2)) {
                        stringstream ss;
                        ss << lua_tonumber(L, -2);

                        lua_append_bson(L, ss.str().c_str(), -1, &b);
                    } else {
                        const char *k = lua_tostring(L, -2);
                        lua_append_bson(L, k, -1, &b);
                    }

                    lua_pop(L, 1);
                }

                builder->append(key, b.obj());
            }
        } else {
            int bson_type = lua_tointeger(L, -1);
            lua_pop(L, 1);
            lua_rawgeti(L, -1, 1);
            switch (bson_type) {
            case mongo::Date:
                builder->appendDate(key, lua_tonumber(L, -1));
                break;
            case mongo::Timestamp:
                builder->appendTimestamp(key);
                break;
            case mongo::RegEx: {
                const char* regex = lua_tostring(L, -1);
                lua_rawgeti(L, -2, 2); // options
                const char* options = lua_tostring(L, -1);
                lua_pop(L, 1);
                builder->appendRegex(key, regex, options);
                break;
                }
            case mongo::NumberInt:
                builder->append(key, static_cast<int32_t>(lua_tointeger(L, -1)));
                break;
            case mongo::NumberLong:
                builder->append(key, static_cast<long long int>(lua_tonumber(L, -1)));
                break;
            case mongo::Symbol:
                builder->appendSymbol(key, lua_tostring(L, -1));
                break;
            case mongo::jstOID: {
                OID oid;
                oid.init(lua_tostring(L, -1));
                builder->appendOID(key, &oid);
                break;
            }
            case mongo::jstNULL:
                builder->appendNull(key);
                break;

            default:
                luaL_error(L, LUAMONGO_UNSUPPORTED_BSON_TYPE, luaL_typename(L, stackpos));
            }
            lua_pop(L, 1);
        }
    } else if (type == LUA_TNIL) {
        builder->appendNull(key);
    } else if (type == LUA_TNUMBER) {
        double numval = lua_tonumber(L, stackpos);
        if (numval == floor(numval)) {
            // The numeric value looks like an integer, treat it as such.
            // This is closer to how JSON datatypes behave.
            int intval = lua_tointeger(L, stackpos);
            builder->append(key, static_cast<int32_t>(intval));
        } else {
            builder->append(key, numval);
        }
    } else if (type == LUA_TBOOLEAN) {
        builder->appendBool(key, lua_toboolean(L, stackpos));
    } else if (type == LUA_TSTRING) {
        builder->append(key, lua_tostring(L, stackpos));
    } else {
        luaL_error(L, LUAMONGO_UNSUPPORTED_LUA_TYPE, luaL_typename(L, stackpos));
    }
}

void bson_to_lua(lua_State *L, const BSONObj &obj) {
    if (obj.isEmpty()) {
        lua_pushnil(L);
    } else {
        bson_to_table(L, obj);
    }
}

// stackpos must be relative to the bottom, i.e., not negative
void lua_to_bson(lua_State *L, int stackpos, BSONObj &obj) {
    BSONObjBuilder builder;

    lua_pushnil(L);
    while (lua_next(L, stackpos) != 0) {
        if (lua_isnumber(L, -2)) {
            stringstream ss;
            ss << lua_tonumber(L, -2);

            lua_append_bson(L, ss.str().c_str(), -1, &builder);
        } else {
            const char *k = lua_tostring(L, -2);
            lua_append_bson(L, k, -1, &builder);
        }

        lua_pop(L, 1);
    }

    obj = builder.obj();
}

const char *bson_name(int type) {
    const char *name;

    switch(type) {
        case mongo::EOO:
            name = "EndOfObject";
            break;
        case mongo::NumberDouble:
            name = "NumberDouble";
            break;
        case mongo::String:
            name = "String";
            break;
        case mongo::Object:
            name = "Object";
            break;
        case mongo::Array:
            name = "Array";
            break;
        case mongo::BinData:
            name = "BinData";
            break;
        case mongo::Undefined:
            name = "Undefined";
            break;
        case mongo::jstOID:
            name = "ObjectID";
            break;
        case mongo::Bool:
            name = "Bool";
            break;
        case mongo::Date:
            name = "Date";
            break;
        case mongo::jstNULL:
            name = "NULL";
            break;
        case mongo::RegEx:
            name = "RegEx";
            break;
        case mongo::DBRef:
            name = "DBRef";
            break;
        case mongo::Code:
            name = "Code";
            break;
        case mongo::Symbol:
            name = "Symbol";
            break;
        case mongo::CodeWScope:
            name = "CodeWScope";
            break;
        case mongo::NumberInt:
            name = "NumberInt";
            break;
        case mongo::Timestamp:
            name = "Timestamp";
            break;
        case mongo::NumberLong:
            name = "NumberLong";
            break;
        default:
            name = "UnknownType";
    }

    return name;
}
