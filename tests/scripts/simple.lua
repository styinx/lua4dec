i = 0
a = "a"
b = 0.123
c = { "a", "b" }
d = { x="a", y="b" }
e = { y=5, z=0.345, a="b" }
f = { 0.345, "b" }
f = { { 0.345 }, "b" }
local g = "b"
h = g
a, b = 8, 5
print(a, b, c)

for j = 0, 20, 5 do
    k = 9
end

while false do
end

repeat
until true

for k, v in e do
end

print()

if i < 5 then
    m = 7
elseif i == 5 then
    n = 8
else
    o = 9
end

closure = function()
    cl = 1
end

function myclosure()
    mycl = 2
end
