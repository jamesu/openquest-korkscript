echo("Hello world");

function emptyLoop(%n) {
  for (%i=0; %i<%n; %i++) { }
}

function fib(%n)
{
   if (%n <= 0)
      return 0;
   if (%n == 1)
      return 1;

   %a = 0;
   %b = 1;

   for (%i = 2; %i <= %n; %i++)
   {
      %c = %a + %b;
      %a = %b;
      %b = %c;
   }

   return %b;
}

function evalInput()
{
	%mx = 0;
	%my = 0;
    if (IsKeyDown(65) || IsKeyDown(263)) %mx -= 1;
    if (IsKeyDown(68) || IsKeyDown(262)) %mx += 1;
    if (IsKeyDown(87) || IsKeyDown(265)) %my -= 1;
    if (IsKeyDown(83) || IsKeyDown(264)) %my += 1;

    if (IsKeyDown(81)) // q
    {
    	TestRoom.setTransitionMode(2, 0, 0.25);
    }

    //fib(7000000);
    SetInput(%mx, %my);
}

exec("scripts/costumes/costumes.cs");

new RootUI(RootUI)
{
};


new Room(TestRoom)
{
	image = "graphics/rooms/back01_merged.bmp";
   boxFile = "graphics/rooms/back01.box";

   new Actor(TestActor)
   {
   };
};
new Room(TestRoom2)
{
   image = "graphics/rooms/titlescreen.bmp";
};
TestActor.setCostume(ZifCostume);

startRoom(TestRoom);
startRoom(TestRoom2);


function TestRoom::inputHandler(%this, %area, %cmd, %btn)
{
   echo("testRoom inputHandler" SPC %this SPC %area SPC %cmd);
   if (%area == 2)
   {
      echo("Mouse click:" SPC $VAR_VIRT_MOUSE_X SPC $VAR_VIRT_MOUSE_Y);
      TestActor.walkTo($VAR_VIRT_MOUSE_X, $VAR_VIRT_MOUSE_Y);
   }
}

function TestRoom::walkAbout(%this, %foo)
{
   echo("walkAbout started" SPC %foo);
   TestActor.setPosition(200, 150);

   while (1)
   {
      //TestActor.walkTo(10,150);
      delayFiber(90);
      echo("walkAbout resumed");
      //TestActor.walkTo(200,150);
      delayFiber(90);
      startRoom(TestRoom);
   }
}

// Start walking thread
$walkFiber = TestRoom.spawnFiber(0, walkAbout, "foo");
echo("Started walking in: " @ $walkFiber);
//TestActor.animate("beam");


$VAR_TIMER_NEXT = 2; // run at 30 fps


