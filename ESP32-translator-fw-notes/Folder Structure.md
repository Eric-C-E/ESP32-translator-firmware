Typical starting folder structure:
projectname/
	CmakeLists.txt
	main/
		CMakeLists.txt
		main.c
		idf_component.yml
	components/

**Folder Structure from Codex**

Codex made a skeleton structure:

This project is supposedly buildable and contains everything in main.

So
ESP32-translator-firmware/
	main/
	app_main.c
	CmakeLists.txt
	Kconfig.projbuild -> adds options into menuconfig
	translator_audio.c -> audio task
	translator_io.c -> GPIO task
	translator_net.c -> net task
	translator_proto.c -> defines fixed-header framing protocol (TCP)
	translator_proto.h -> exposes it
	translator_runtime.c -> builds shared runtime state (queues + event groups)
	translator_runtime.h -> exposes it
	translator_tasks.h
	translator_ui.c -> UI task
gitignore
CmakeLists.txt
License
Readme
TODO text file

So the pattern is, the CMakeLists is MANDATORY, and files to run go into main folder. 

	