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

line_data myline;
std::vector<sign_data> mysigns;
Engine engine;
System syst;
Client *client;

int main(int argc, char* argv[])
{
    /* GUI initialization */
	Platform wi;
    struct nk_context* ctx;
    
	windowInitialization(&wi, "Client", WINDOW_WIDTH, WINDOW_HEIGHT);
	ctx = ctxInitialization(&wi, WINDOW_WIDTH, WINDOW_HEIGHT);
	loadDefaultFont(&wi);

    glEnable(GL_TEXTURE_2D);
    struct nk_image streamImage;
    struct nk_image stopSign = loadImageFromFile("../images/stop.jpeg");
    struct nk_image giveWaySign = loadImageFromFile("../images/ustupi.jpg");
    struct nk_image mainRoadSign = loadImageFromFile("../images/glavnaya.jpg");
    struct nk_image crosswalkSign = loadImageFromFile("../images/crosswalk.jpeg");
    struct nk_image greenLight = loadImageFromFile("../images/green_light.jpg");
    struct nk_image yellowLight = loadImageFromFile("../images/yellow_light.jpg");
    struct nk_image redLight = loadImageFromFile("../images/red_light.jpg");

    /* Client initialization */
    Object<std::vector<unsigned char>> *curObj = NULL;
    Queue<std::vector<unsigned char>> &queue = syst.iqueue;
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

	while (true) 
    {
        syst.line_get(myline);
	    syst.signs_get(mysigns);
	    syst.engine_get(engine);
        
        /* take new obj */
        curObj = queue.waitForNewObject(curObj);
        unsigned char* buff = new unsigned char[curObj->obj->size()];
        for (int i = 0; i < curObj->obj->size(); ++i)
            buff[i] = (*(curObj->obj))[i];
        streamImage = loadStreamImageFromMemory(buff, sizeof(unsigned char) * curObj->obj->size(), myline, mysigns);        
        delete[] buff;
        
        /* Input */
        SDL_Event evt;
        nk_input_begin(ctx);
        while (SDL_PollEvent(&evt)) {
            if (evt.type == SDL_QUIT) {
                shutdown(&wi);
                return 0;
            }
            nk_sdl_handle_event(&evt);
        }
        nk_input_end(ctx);

        /* GUI */
        if (nk_begin(ctx, "Stream", nk_rect(0, 0, WINDOW_WIDTH - 300, WINDOW_HEIGHT), NK_WINDOW_TITLE)) 
        {
            nk_layout_row_static(ctx, WINDOW_HEIGHT - 50, WINDOW_WIDTH - 322, 1);
            nk_image(ctx, streamImage);
        }
        nk_end(ctx);

        if (nk_begin(ctx, "Info", nk_rect(WINDOW_WIDTH - 300, 0, 300, WINDOW_HEIGHT), NK_WINDOW_TITLE)) 
        {
            for (unsigned i = 0; i < mysigns.size(); ++i) 
            {
		        switch (mysigns[i].sign)
                {
                    case sign_none:
                        break;
                    case sign_crosswalk:
                        nk_layout_row_static(ctx, 100, 100, 1);
                        nk_image(ctx, crosswalkSign);			
                        break;
                    case sign_stop: 
                        nk_layout_row_static(ctx, 100, 100, 1);                       
                        nk_image(ctx, stopSign);
                        break;
                    case sign_mainroad:
                        nk_layout_row_static(ctx, 100, 100, 1);
                        nk_image(ctx, mainRoadSign);
                        break;
                    case sign_giveway:
                        nk_layout_row_static(ctx, 100, 100, 1);
                        nk_image(ctx, giveWaySign);
                        break;
                    case sign_trafficlight_red:
                        nk_layout_row_static(ctx, 100, 100, 1);
                        nk_image(ctx, redLight);
                        break;
                    case sign_trafficlight_yellow:
                        nk_layout_row_static(ctx, 100, 100, 1);
                        nk_image(ctx, yellowLight);
                        break;
                    case sign_trafficlight_green:
                        nk_layout_row_static(ctx, 100, 100, 1);
                        nk_image(ctx, greenLight);
                        break;
                    case sign_starttrafficlight_red:
                        
                        break;
                    case sign_starttrafficlight_green:
                        
                        break;
                    default:
                        break;
                }
            }
        }
        nk_end(ctx);

		render(&wi);

        curObj->free();        
        glDeleteTextures(1,(const GLuint*)&streamImage.handle.id);        
	}

	shutdown(&wi);
    syst.setExitState();

	return 0;
}
