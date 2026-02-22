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

$NORTH = 0;
$SOUTH = 1;
$WEST = 2;
$EAST = 3;

new Room(TestRoom)
{
	image = "graphics/rooms/back01_merged.bmp";
   boxFile = "graphics/rooms/back01.box";
   zPlane[0]  = "graphics/rooms/back01_mask1.bmp";
   zPlane[1] = "graphics/rooms/back01_mask2.bmp";
   zPlane[2] = "graphics/rooms/back01_mask2.bmp";

   new Actor(TestActor)
   {
   };

    new RoomObject(plant)
    {
        anchorPoint = 104, 48;
        dir = $EAST;
        descName = "plant";
        state = 2;
        
        new RoomObjectState()
        {
           hotSpot = 0, 48;
           image = "graphics/background_items/plant_unmoved.bmp";

           zPlane[0] = "";
           zPlane[1] = "graphics/background_items/plant_mask2.bmp";
           zPlane[2] = "graphics/background_items/plant_unmoved_mask3.bmp";
         };


        new RoomObjectState()
        {
           hotSpot = 0, 48;
           image = "graphics/background_items/plant_moved.bmp";

           zPlane[0] = "";
           zPlane[1] = "graphics/background_items/plant_mask2.bmp";
           zPlane[2] = "graphics/background_items/plant_moved_mask3.bmp";
        };
    };

};
new Room(TestRoom2)
{
   image = "graphics/rooms/titlescreen.bmp";
};

new RoomObject(TestVerbRootBg)
{
  anchorPoint = 0, 144;
  state = 1;
  
  new RoomObjectState()
  {
     image = "graphics/verbs/verb_background.bmp";
  };
};

new VerbDisplay(TestVerbRoot)
{
   roomObject = TestVerbRootBg;
   displayText = Test;
   anchorPoint = 0, 144;
};

new VerbDisplay(TestGiveVerb)
{
   displayText = "Give";
   anchorPoint = 146, 174;
   center = true;
   enabled = true;
   hotKey = $KEY_G;
};


new Sound(Test_beamedSnd)       { path = "sounds/loading_gun.wav"; };

// ...

TestActor.setCostume(ZifCostume);

startRoom(TestRoom);
startRoom(TestRoom2);

RootUI.add(TestVerbRoot);
RootUI.add(TestGiveVerb);

function TestRoom::inputHandler(%this, %area, %cmd, %btn)
{
   echo("testRoom inputHandler" SPC %this SPC %area SPC "cmd=" @ %cmd SPC "btn=" @ %btn);
   if (%area == 2)
   {
      echo("Mouse click:" SPC $VAR_VIRT_MOUSE_X SPC $VAR_VIRT_MOUSE_Y);
      TestActor.walkTo($VAR_VIRT_MOUSE_X, $VAR_VIRT_MOUSE_Y);
   }
   else if (%area == 4)
   {
      if (%btn == 81)
      {
         TestRoom.setTransitionMode(2, 0, 0.25);
      }
      else if (%btn == 87)
      {
         TestRoom.setTransitionMode(1, 0, 0.25);
      }
   }
}

function TestRoom::walkAbout(%this, %foo)
{
   echo("walkAbout started" SPC %foo);
   TestActor.setPosition(200, 150);

   while (1)
   {
      //TestActor.walkTo(10,150);
      TestActor.say("testing");
      delayFiber(90);
      TestActor.say("testing...");
      echo("walkAbout resumed");
      //TestActor.walkTo(200,150);
      delayFiber(90);
      startRoom(TestRoom);

      Test_beamedSnd.play();
   }
}

// Start walking thread
$walkFiber = TestRoom.spawnFiber(0, walkAbout, "foo");
echo("Started walking in: " @ $walkFiber);
//TestActor.animate("beam");


$VAR_TIMER_NEXT = 2; // run at 30 fps


