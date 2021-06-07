Console.log("math.js");
let nm = NekoMath;

let poop = 1337;
let asd = ( poop * 3 );
Console.print(["aint that some shit?!!", asd, poop].join(" "));
Console.print("let's test the vector thingy");
let x = new vec2(1,1);
let y = vec2(2,2);
Console.print(x);
Console.print(y);
Console.print("greater than:");
Console.print(x.gt(y));
Console.print(nm.gt(y,x));
Console.print("less than:");
Console.print(nm.lt(x,y));
Console.print(y.lt(x));
Console.print("equal:");
Console.print(nm.eq(x,y));
x = vec2(2,2);
Console.print(y.equals(x));

Console.print("NEW SHIT");
const qwe = vec3(1, 2, 3);
const rty = vec3(5, 6, 7);
Console.print("addition: " + nm.add(qwe, rty));
Console.print("subtraction: " + nm.sub(qwe, rty));