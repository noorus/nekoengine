const nm = NekoMath;

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

class TestScene extends Scene
{
  constructor( name )
  {
    super( name );
    this._camera = null;
  }
  initialize( time )
  {
    Console.print( "TestScene.initialize: " + time );
    this._camera = Game.getEntity( "gamecam" );
    Console.dump( this._camera );
    Console.dump( this._camera.transform );
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
    this._camera.transform.rotate.fromAngleAxis( nm.radians( time * 50 ), vec3( 0, 0, 1 ) );
  }
}

Game.registerScene( new TestScene( "demo" ) );
