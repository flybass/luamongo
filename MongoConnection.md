# Synopsis #


```
require('mongo')

-- Create a connection object
local db = assert(mongo.Connection.New())

-- connect to the server on localhost
assert(db:connect('localhost'))

-- insert a value into the namespace 'test.values'
assert(db:insert('test.values', {a = 10, b = 'str1'}))

-- the same using a JSON string
assert(db:insert('test.values', "{'a': 20, 'b': 'str2'}"))

-- insert a multiple values into the namespace 'test.values'
assert(db:insert_batch('test.values', {{a = 10, b = 'str1'}, {c = 11, d = 'str2'}}))

-- print the number of rows in the namespace 'test.values'
print(db:count('test.values'))

-- query all the values in the namespace 'test.values'
local q = assert(db:query('test.values', {}))

-- loop through the result set
for result in q:results() do
        print(result.a)
        print(result.b)
end


```


## Methods ##

### db,err = mongo.Connection.New(t) ###
Returns a new Connection object, or nil and an error.
Accepts an optional table of features:
  * **auto\_reconnect**,   (default = false)
  * **rw\_timeout**,       (default = 0, only supported in mongo >= v1.5)

### ok,err = db:connect(connection\_str) ###

### ok,err = db:auth(t) ###
Authenticates a given database using a table of parameters:
  * **dbname**,           database to authenticate (required)
  * **username**,         username to authenticate against (required)
  * **password**,         password to authenticate against (required)
  * **digestPassword**,   if password is plain text, set this to true. otherwise assumed to be pre-digested

### count,err = db:count(namespace) ###

### ok,err = db:insert(namespace, value) ###

### ok,err = db:insert(namespace, {value1, value2, ...}) ###
Inserts multiple documents in one system call.

### cursor,err = db:query(namespace, query) ###
TODO: [document more options](http://code.google.com/p/luamongo/source/browse/trunk/mongo_connection.cpp#206)

### ok,err = db:remove(namespace, query, single) ###

### ok,err = db:update(namespace, query, modifier, upsert, multi) ###

### is\_failed = db:is\_failed() ###

### addr = db:get\_server\_address() ###