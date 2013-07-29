
Uint32 getPixel(SDL_Surface *surface, int x, int y)
{
    int bpp = surface->format->BytesPerPixel;
     /* Ici p est l'adresse du pixel que l'on veut connaitre */
     Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;


         return *(Uint16 *)p;
 }
 
void SetPixel(SDL_Surface* Surface, int x, int y, Uint32 pixel)
{
	int bpp = Surface->format->BytesPerPixel;
    /* p est l'adresse du pixel que l'on veut modifier */
    Uint8 *p = (Uint8*)Surface->pixels + y * Surface->pitch + x * bpp;

        *(Uint16*)p = pixel;
}

//creation bg ombré
void pixel_gamma( SDL_Surface *Surface )
{
	Uint32 pixel;
	Uint8 r,g,b,a;
	int x,y;
	// precalcul palette
	unsigned char precal_pal[256];
	
	capex.FLshadow = (float) capex.FXshadow / 100 ;
	for(x=255;x>-1;--x) precal_pal[x]=x*capex.FLshadow;
	
	for(y=479;y>-1;y--){
		for(x=639;x>-1;x--){
			pixel=getPixel(Surface,x,y);
			SDL_GetRGBA(pixel, Surface->format, &r, &g, &b, &a);
			//Ici, on mettra du code pour modifier les pixels.
			if(r) r=precal_pal[r];
			if(g) g=precal_pal[g];
			if(b) b=precal_pal[b];
			//Et une fois qu'on les a modifiés :
			pixel=SDL_MapRGBA(Surface->format, r, g, b, a);
			//Et pour changer la valeur d'un pixel :
			SetPixel(Surface,x,y,pixel);
		}
	}
}
