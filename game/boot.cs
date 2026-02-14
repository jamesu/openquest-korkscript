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

new Room(TestRoom)
{
	image = "graphics/rooms/back01_merged.bmp";
   boxFile = "graphics/rooms/back01.box";

   new Actor(TestActor)
   {
   };
};

new RootUI(RootUI)
{
};

RootUI.setContent(TestRoom);

TestActor.setCostume(ZifCostume);

function walkAbout()
{
   echo("walkAbout started");
   TestActor.setPosition(200, 150);

   while (1)
   {
      TestActor.walkTo(10,150);
      delayFiber(90);
      echo("walkAbout resumed");
      TestActor.walkTo(200,150);
      delayFiber(90);
   }
}

// Start walking thread
$walkFiber = spawnFiber(walkAbout);
echo("Started walking in: " @ $walkFiber);
//TestActor.animate("beam");


$VAR_TIMER_NEXT = 2; // run at 30 fps


