local x = 0
local w = 1

c = x
d = w
e = c or d

function closure(f, e)
    local b = 2
    local a = 3

    local c, d = dummy()

    for i = 0, 9, 14 do
        local e = nil
        print(c)
    end

    print(a)

    i = g
    j = h

    return print(b)
end
