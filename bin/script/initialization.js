/*let poop = 1337;
let asd = ( poop * 3 );
Console.print(["aint that some shit?!!", asd, poop].join(" "));
Console.print("let's test the vector thingy");
let x = new vec2(1,1);
let y = vec2(2,2);
Console.print(x);
Console.print(y);
let nm = NekoMath;
Console.print("greater than:");
Console.print(x.gt(y));
Console.print(nm.gt(y,x));
Console.print("less than:");
Console.print(nm.lt(x,y));
Console.print(y.lt(x));
Console.print("equal:");
Console.print(nm.eq(x,y));
x = vec2(2,2);
Console.print(y.equals(x));*/

class Scene {
  constructor( name )
  {
    this._name = name;
  }
  get name()
  {
    return this._name;
  }
}

class DemoScene extends Scene {
  constructor( name )
  {
    super(name);
  }
  initialize(time)
  {
    Console.print("SCRIPT: "+this._name+" INITIALIZE "+time);
  }
  update(time, delta)
  {
  }
}

let demo = new DemoScene("demo");
Game.registerScene(demo);