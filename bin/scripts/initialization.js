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
    /*Console.print( this._text );
    Console.print( "ENTTEST" );
    const gc = Game.getEntity( "gamecam" );
    Console.print( gc );
    this._enttest = new TransformComponent( 1 );
    Console.print( this._enttest );*/
    this._camera = Game.getEntity( "gamecam" ); // Game.createEntity( "loloooooooool" );
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
    //this._enttest.translate.x = Math.sin( time * 2 ) * 2;
//this._enttest.translate.y = Math.cos( time * 2 ) * 2;
   // this._enttest.rotate.fromAngleAxis( nm.radians( time * 50 ), vec3( 0, 0, 1 ) );
    this._camera.transform.rotate.fromAngleAxis( nm.radians( time * 50 ), vec3( 0, 0, 1 ) );
  }
}

Game.registerScene( new TestScene( "demo" ) );
