#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <thread>
#include <vector>

#include "CLP.hpp"
#include "signs.hpp"
#include "client.hpp"
#include "config.hpp"
#include "Engine.hpp"
#include "guiFunctions.h"

System syst;
Engine engine;
Client *client;
line_data myline;
std::vector<sign_data> mysigns;

int main(int argc, char* argv[])
{
	/* GUI initialization */
	Platform wi;
	struct nk_context* ctx;

	windowInitialization(&wi, "Client", WINDOW_WIDTH, WINDOW_HEIGHT);
	ctx = ctxInitialization(&wi, WINDOW_WIDTH, WINDOW_HEIGHT);
	loadDefaultFont(&wi);

	#if defined(UNIX)
		glEnable(GL_TEXTURE_2D);
	#endif

	struct nk_image streamImage;
	struct nk_image stopSignImage        = loadImageFromFile("../images/stop.jpeg");
	struct nk_image giveWaySignImage     = loadImageFromFile("../images/ustupi.jpg");
	struct nk_image mainRoadSignImage    = loadImageFromFile("../images/glavnaya.jpg");
	struct nk_image crosswalkSignImage   = loadImageFromFile("../images/crosswalk.jpeg");
	struct nk_image mainGreenLightImage  = loadImageFromFile("../images/green_light.jpg");
	struct nk_image mainYellowLightImage = loadImageFromFile("../images/yellow_light.jpg");
	struct nk_image mainRedLightImage    = loadImageFromFile("../images/red_light.jpg");
	struct nk_image startRedLightImage   = loadImageFromFile("../images/st_red_light_s.jpg");
	struct nk_image startGreenLightImage = loadImageFromFile("../images/st_green_light_s.jpg");

	/* Client initialization */
	Object< std::vector<unsigned char> > *curObj = NULL;
	Queue< std::vector<unsigned char> > &queue = syst.iqueue;
	CLP::parse(argc, argv, syst);

	client = new Client(syst);
	
	printf("[I]: Connecting to %s:%d...\n",syst.host, syst.portno);
	
	if(!client->connect()) {
		printf("[E]: Connection failed.\n");
		printf("Can't connect to server.\n");
        	return 1;
	}
	printf("Connection was successfully established!\n");

	std::thread thr(client_fnc,ref(syst),ref(*client));
	thr.detach();

	int receivedPower;
	char enginePower[] = "Power: xx%";

	#if defined(WIN32)
		MSG msg;
		int running;
		int needs_refresh;
	#elif defined(UNIX)
		SDL_Event evt;
	#endif

	while (true)
	{
		/* Input */
		#if defined(WIN32)
			nk_input_begin(ctx);
			if (needs_refresh == 0) {
				if (GetMessageW(&msg, NULL, 0, 0) <= 0) {
					running = 0;
				} else {
					TranslateMessage(&msg);
					DispatchMessageW(&msg);
				}
				needs_refresh = 1;
			} else {
				needs_refresh = 0;
			}

			while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
				if (msg.message == WM_QUIT) {
					running = 0;
					shutdown(&wi);
					return 0;
				}
				TranslateMessage(&msg);
				DispatchMessageW(&msg);
				needs_refresh = 1;
			}
			nk_input_end(ctx);
		#elif defined(UNIX)
			nk_input_begin(ctx);
			while (SDL_PollEvent(&evt)) {
				if (evt.type == SDL_QUIT) {
					shutdown(&wi);
					return 0;
				}
				nk_sdl_handle_event(&evt);
			}
			nk_input_end(ctx);
		#endif

		/* Get RoboSystem data */
		syst.line_get(myline);
		syst.signs_get(mysigns);
		syst.engine_get(engine);
        
        	/* Get new obj */
		curObj = queue.waitForNewObject(curObj);
		unsigned char* buff = new unsigned char[curObj->obj->size()];
		for (int i = 0; i < curObj->obj->size(); ++i)
        		buff[i] = (*(curObj->obj))[i];
		streamImage = loadImageFromMemory(buff, sizeof(unsigned char) * curObj->obj->size(), myline, mysigns, streamImageFunc);
		delete[] buff;

        	/* Draw GUI */
		if (nk_begin(ctx, "Stream", nk_rect(0, 0, WINDOW_WIDTH - 300, WINDOW_HEIGHT), NK_WINDOW_TITLE)) 
		{
			nk_layout_row_static(ctx, WINDOW_HEIGHT - 50, WINDOW_WIDTH - 322, 1);
			nk_image(ctx, streamImage);
		}
		nk_end(ctx);

		if (nk_begin(ctx, "Objects", nk_rect(WINDOW_WIDTH - 300, 0, 300, WINDOW_HEIGHT - 100), NK_WINDOW_TITLE)) 
		{
			for (unsigned i = 0; i < mysigns.size(); ++i) 
			{
				switch (mysigns[i].sign) {
					case sign_none:
						break;
					case sign_crosswalk:
						nk_layout_row_static(ctx, 100, 100, 1);
						nk_image(ctx, crosswalkSignImage);			
						break;
					case sign_stop: 
						nk_layout_row_static(ctx, 100, 100, 1);
						nk_image(ctx, stopSignImage);
						break;
					case sign_mainroad:
						nk_layout_row_static(ctx, 100, 100, 1);
						nk_image(ctx, mainRoadSignImage);
						break;
					case sign_giveway:
						nk_layout_row_static(ctx, 100, 100, 1);
						nk_image(ctx, giveWaySignImage);
						break;
					case sign_trafficlight_red:
						nk_layout_row_static(ctx, 200, 100, 1);
						nk_image(ctx, mainRedLightImage);
						break;
					case sign_trafficlight_yellow:
						nk_layout_row_static(ctx, 200, 100, 1);
						nk_image(ctx, mainYellowLightImage);
						break;
					case sign_trafficlight_green:
						nk_layout_row_static(ctx, 200, 100, 1);
						nk_image(ctx, mainGreenLightImage);
						break;
					case sign_starttrafficlight_red:
						nk_layout_row_static(ctx, 200, 100, 1);
						nk_image(ctx, startRedLightImage);
						break;
					case sign_starttrafficlight_green:
						nk_layout_row_static(ctx, 200, 100, 1);
						nk_image(ctx, startGreenLightImage);
						break;
					default:
						break;
				}
			}
		}
		nk_end(ctx);

		if (nk_begin(ctx, "Info", nk_rect(WINDOW_WIDTH - 300, WINDOW_HEIGHT - 100, 300, 100), NK_WINDOW_TITLE)) 
		{
			receivedPower = engine.speed / 10;
			enginePower[7] = receivedPower / 10 % 10 + '0';
			enginePower[8] = receivedPower % 10 + '0';
			nk_layout_row_static(ctx, 30, 100, 1);
			nk_label(ctx, enginePower, NK_TEXT_LEFT);
		}
		nk_end(ctx);

		render(&wi);
        
		curObj->free();
		#if defined(UNIX)
			glDeleteTextures(1, (const GLuint*) &streamImage.handle.id);
		#endif
	}

	shutdown(&wi);
	syst.setExitState();

	return 0;
}
