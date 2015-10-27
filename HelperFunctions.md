# Synopsis #

```

require('mongo')

local date = mongo.Date(os.time() * 1000)
local newint = mongo.NumberInt(123)
local rx = mongo.RegEx('abc', 'gi')

print(mongo.type(date))             -- prints 'mongo.Date'
print(mongo.type(rx))               -- prints 'mongo.RegEx'
print(mongo.type({}))               -- prints 'table'
print(mongo.type('test'))           -- prints 'string'

local x = tonumber(newint)  + 10    -- x = 133
print(tonumber(rx))                 -- prints 'nil'
print(tonumber('test'))             -- prints 'nil'
print(tonumber('FF', 16))           -- prints '255'

```

## Functions ##

### `typename = mongo.type(obj)` ###

Returns a string name of the BSON type of `obj`. If `obj` is not a BSON object, returns the standard Lua type name.

### `num = mongo.tonumber(obj[, base])` ###

Returns the number value represented by BSON data `obj`. If the value of `obj` cannot be converted into a number returns `nil`.

If `obj` is not a BSON data object behaves exactly the same as Lua's builtin `tonumber`.