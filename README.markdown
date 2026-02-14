# openquest-korkscript

openquest-korkscript is an port of openquest from scummc which in turn is a port from AGS.

It's basically an implementation of a scumm-style adventure game combined with an original runtime.

Game scripts are written in korkscript.

This project is still WIP; neither the runtime and the script code have been completed to a playable state.

## Compiling

Project files are generated with cmake.

Simple test app build:

	mkdir build
	cd build
	cmake ..
	make

## License

Code located in the game folder is a port from the original scummc code, thus is licensed under GPL v2.

All other code (i.e. in the root folder and src) should be considered licensed under the MIT license by their respective authors unless otherwise specified. Main license text is in LICENSE.
raygui is also included; this uses a zlib/libpng license.
