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

class Scene
{
  constructor( name )
  {
    this._name = name;
  }
  get name()
  {
    return this._name;
  }
}

class DemoScene extends Scene
{
  constructor( name )
  {
    super( name );
    this._mesh = null;
    this._model = null;
  }
  initialize( time )
  {
    Console.print( "DemoScene.initialize: " + time );
    /*const verts = [
      [-128,  90,  90, 0, 0.7, -0.7, 0, 0],
      [-128, -90, -90, 0, 0.7, -0.7, 0, 1],
      [128, 90, 90, 0, 0.7, -0.7, 1, 0],
      [128, -90, -90, 0, 0.7, -0.7, 1, 1],
    ];
    const indices = [
      0, 2, 1, 1, 2, 3
    ];
    this._mesh = new mesh(verts, indices);*/
    this._mesh = new mesh( "plane", vec2( 256 ), vec2( 1 ), vec3( 0, 0, -1 ) );
    this._ctr = 0;
    Console.print(this._mesh);
    Console.print("creating model");
    this._model = new model({
      mesh: this._mesh,
      scale: vec3( 1, 1, 1 ),
      translate: vec3( 640, 360, 0 )
    });
    Console.print(this._model);
    Console.print("DemoScene.initialize leaving function" );
  }
  enter()
  {
    Console.print( "DemoScene.enter" );
  }
  leave()
  {
    Console.print( "DemoScene.leave" );
  }
  update( time, delta )
  {
    const scalar = 2 + Math.sin(time * 2) * 1.5;
    // Console.print("Scalar " + scalar);
    this._model.scale = vec3(scalar);
    /*if (this._ctr < 12) {
      Console.print("update " + this._ctr + " - time " + time);
      Console.print(this._mesh);
    }
    if (this._ctr == 11) {
      this._mesh = null;
    }
    this._ctr++;*/
  }
}

let demoscene = new DemoScene( "demo" );
Game.registerScene( demoscene );