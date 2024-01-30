function foo()
    a = 1
    return a
end

function bar()
    a = 1
    b = 2
    return a, b
end

function baz()
    a = 1
    return
end

foobar = function(x)
    return function(y) return y end
end

barbaz = function(z)
    a = 99
    if z == 0 then
        return z
    else
        return z + 1
    end
    b = 12
    return a, b
end

return bar()
