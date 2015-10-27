# Synopsis #


```
require('mongo')
-- query all the values in the namespace 'test.values' where a > 10
local q1 = assert(db:query('test.values', {a = {['$gt'] = 10}}))

-- loop through the result set
for result in q1:results() do
        print(result.a)
        print(result.b)
end

-- query all the values in the namespace 'test.values' where a < 10 (JSON version)
local q2 = assert(db:query('test.values', "{'a': {'$lt': 10}}"))

local result = q2:next()
while result do
        print(result.a)
        print(result.b)

        result = q2:next()
end

```


## Methods ##

### result = query:next() ###

Returns the next single result set (as a table) in the results of a query operation.

### iterator = query:results() ###

Similar to the `next` method but returns an iterator function (as opposed to a table) that can be used in a for loop.

### has\_more = cursor:has\_more(in\_current\_batch) ###
  * pass true to call moreInCurrentBatch (mongo >=1.5)

### it\_count = cursor:itcount() ###

### is\_dead = cursor:is\_dead() ###

### is\_tailable = cursor:is\_tailable() ###

### has\_result\_flag = cursor:has\_result\_flag() ###

### id = cursor:get\_id() ###