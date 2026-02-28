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
exec("scripts/common.cs");

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
   enabled = false;
};

new SimSet(TestVerbSet) {

    new VerbDisplay([WalkTo]) {
    
       displayText = "Walk to";
       hotKey = "w";
    };
    
    new VerbDisplay([Give]) {
    
       displayText = "Give";
       anchorPoint = 146, 174;
       //backColor = %backColor;
       //color = %color;
       //hiColor = %hiColor;
       //dimColor = %dimColor;
       center = 1;
       enabled = 1;
       hotKey = "g";
        isPreposition = true;
    };

    new VerbDisplay([PickUp]) {
    
       displayText = "Pick up";
       anchorPoint = 102, 161;
       //backColor = %backColor;
       //color = %color;
       //hiColor = %hiColor;
       //dimColor = %dimColor;
       center = 1;
       enabled = 1;
       hotKey = "p";
        isPreposition = true;
    };

    new VerbDisplay([Use]) {
    
       displayText = "Use";
       anchorPoint = 146, 187;
       //backColor = %backColor;
       //color = %color;
       //hiColor = %hiColor;
       //dimColor = %dimColor;
       center = 1;
       enabled = 1;
       hotKey = "u";
        isPreposition = true;
    };

    new VerbDisplay([Open]) {
    
       displayText = "Open";
       anchorPoint = 188, 161;
       //backColor = %backColor;
       //color = %color;
       //hiColor = %hiColor;
       //dimColor = %dimColor;
       center = 1;
       enabled = 1;
       hotKey = "o";
        isPreposition = true;
    };

    new VerbDisplay([LookAt]) {
    
       displayText = "Examine";
       anchorPoint = 146, 161;
       //backColor = %backColor;
       //color = %color;
       //hiColor = %hiColor;
       //dimColor = %dimColor;
       center = 1;
       enabled = 1;
       hotKey = "e";
        isPreposition = true;
    };

    new VerbDisplay([Smell]) {
    
       displayText = "Smell";
       anchorPoint = 188, 174;
       //backColor = %backColor;
       //color = %color;
       //hiColor = %hiColor;
       //dimColor = %dimColor;
       center = 1;
       enabled = 1;
       hotKey = "s";
        isPreposition = true;
    };

    new VerbDisplay([TalkTo]) {
    
       displayText = "Talk to";
       anchorPoint = 102, 174;
       //backColor = %backColor;
       //color = %color;
       //hiColor = %hiColor;
       //dimColor = %dimColor;
       center = 1;
       enabled = 1;
       hotKey = "t";
        isPreposition = true;
    };

    new VerbDisplay([Move]) {
    
       displayText = "Move";
       anchorPoint = 188, 187;
       //backColor = %backColor;
       //color = %color;
       //hiColor = %hiColor;
       //dimColor = %dimColor;
       center = 1;
       enabled = 1;
       hotKey = "m";
        isPreposition = true;
    };

    // Inventory scrollers
    new VerbDisplay([invUp]) {
       displayText = "\x03";     // up arrow glyph
          anchorPoint = 309, 165;
       //backColor = %backColor;
       //color = %color;
       //hiColor = %hiColor;
       //dimColor = %dimColor;
    };

    new VerbDisplay([invDown]) {
       displayText = "\x02";     // down arrow glyph
          anchorPoint = 309, 185;
       //backColor = %backColor;
       //color = %color;
       //hiColor = %hiColor;
       //dimColor = %dimColor;
    };
};

new Sound(Test_beamedSnd)       { path = "sounds/loading_gun.wav"; };

// ...

TestActor.setCostume(ZifCostume);

startRoom(TestRoom);
startRoom(TestRoom2);

RootUI.add(TestVerbRoot);

foreach (%verb in TestVerbSet)
{
   echo("FORECH", %verb);
   RootUI.add(%verb);
}

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

      echo("**should stop run for a short while...");
      pushFiberSuspendFlags(0x4);
      TestActor.say("testing...");
      echo("walkAbout resumed");
      //TestActor.walkTo(200,150);
      delayFiber(90);
      startRoom(TestRoom);
      popFiberSuspendFlags();
      echo("**should run for a short while...");

      Test_beamedSnd.play();
   }
}

echo("TEST SQM");

$dHandler = new VerbDisplay([testHandler]) {
};

function DisplayBase::onTestHandler(%this, %vrb, %objA, %objB)
{
   echo("onTestHandler called:" SPC %vrb SPC %obja SPC %objB);
   nop();
}

echo("ze push");
SentenceQueue.push($dHandler, $dHandler, 0);


function testRunWhileMask()
{
   while (1)
   {
      delayFiber(10);
      echo("running");
   }
}

spawnFiber(0x4, testRunWhileMask);


// Start walking thread
$walkFiber = TestRoom.spawnFiber(0, walkAbout, "foo");
echo("Started walking in: " @ $walkFiber);
//TestActor.animate("beam");


$VAR_TIMER_NEXT = 2; // run at 30 fps


// Cutscenes:
// beginCutscene -> push cutscene fiber
// endCutscene -> pop stack 




