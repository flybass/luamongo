# Synopsis #


```
require('mongo')

-- Create a connection object
local db = assert(mongo.Connection.New())

-- connect to the server on localhost
assert(db:connect('localhost'))

-- create a GridFS handle
local gridfs = assert(mongo.GridFS.New(db, 'test'))

-- store a file on the server
local gridfile = assert(gridfs:store_file('/path/to/file'))

-- print details
print(gridfile:num_chunks())
print(gridfile:chunk_size())
print(gridfile:md5())
print(gridfile:metadata())
print(gridfile:upload_date())
print(#gridfile) -- synonym for gridfile:content_length()

-- examine file chunks
local chunk = assert(gridfile:chunk(0))
print(#chunk) -- synonym for chunk:len()
print(chunk:data())

-- write file data
assert(gridfile:write('/path/to/newfile'))

-- list files on the DB
local q = gridfs:list()
for gf in q:results() do
    print(gf:field('filename'))
end

```

## Methods ##

### `gridfs, err = mongo.GridFS.New(connection, dbname[, prefix])` ###
### `gridfile, err = gridfs:find_file(filename)` ###
### `cursor = gridfs:list()` ###
### `ok, err = gridfs:remove_file(filename)` ###
### `gridfile, err = gridfs:store_file(filename[, remote_file[, content_type]])` ###

### `chunk, err = gridfile:chunk(chunk_num)` ###
### `chunk_size = gridfile:chunk_size()` ###
### `content_length = gridfile:content_length()` ###
### `bool = gridfile:exists()` ###
### `filename_str = gridfile:filename()` ###
### `md5_str = gridfile:md5()` ###
### `metadata_table = gridfile:metadata()` ###
### `num_chunks = gridfile:num_chunks()` ###
### `date = gridfile:upload_date()` ###
### `success,err = gridfile:write(filename)` ###

### `str = chunk:data()` ###
### `length = chunk:len()` ###