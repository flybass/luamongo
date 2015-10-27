luamongo automatically marshals data between [Lua types](http://www.lua.org/manual/5.1/manual.html#2.2) and [BSON types](http://www.mongodb.org/display/DOCS/BSON).  Here is the mapping:

| **Lua Type** | **BSON Type** | **Notes** |
|:-------------|:--------------|:----------|
| nil          | Null value    |           |
| boolean      | Boolean       |           |
| number       | Floating point |           |
| string       | String        |           |
| table        | new BSON Document by iterating pairs |           |
| mongo.Date   | UTC Datetime  | milliseconds since epoch, 64-bit, precision lost |
| mongo.Timestamp | Timestamp     | special mongodb type, 64-bit, precision lost |
| mongo.RegEx  | Regular Expression |           |
| mongo.NumberInt | 32-bit integer |           |
| mongo.NumberLong | 64-bit integer | precision lost |
| mongo.Symbol | Symbol        |           |
| mongo.ObjectID | ObjectID      | [MongoDB document ID](http://www.mongodb.org/display/DOCS/Object+IDs) |
| thread       | UNSUPPORTED   |           |
| function     | UNSUPPORTED   |           |
| userdata     | UNSUPPORTED   |           |


---

## How To Use BSON Types ##

You use the non-native types as follows:

```
-- store 'foo' as an integer, and 'bar' as a float
db:insert('test.values', { foo = mongo.NumberInt(5), bar = 5 }
```


---


## 64-bit Integer Precision Warning ##

Many BSON types are natively 64-bit integers.  However, Lua numbers only support 52-bits of integer precision.  Thus, if you convert these BSON types into Lua, you may lose precision.


---


## Implementation Notes ##

The various BSON type objects (e.g. mongo.Date) work by creating a table with the following structure:
```
t[1] = value
getmetatable(t).__bsontype = numeric_code_of_bson_type
```

When the marshalling code sees a table with this metatable field, it will change how it marshals the object.  You can find this [here in the source code](http://code.google.com/p/luamongo/source/browse/trunk/utils.cpp#90).