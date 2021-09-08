Console.log("initialization.js");

// include("math.js");

Console.log("include done");

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

let toRadians = (degrees) => {
  return (degrees * (Math.PI / 180));
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
    this._mesh = new mesh("ground", vec3(16, 16, 0.2), vec2(32));
    this._ctr = 0;
    Console.print(this._mesh);
    Console.print("creating model");
    this._models = [
      new model({
        mesh: this._mesh,
        scale: vec3(1, 1, 1),
        translate: vec3(0, -0.5, 0)
      })
    ]
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
    //this._models[1].rotate.fromAngleAxis(toRadians(time * 50), vec3(0, 0, 1));
    //this._models[2].translate.z = Math.sin(time * 2) * 2;
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