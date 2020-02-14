#include "GB_gameboy.h"
#include "GB_video.h"
#include "GB_memory.h"
#include <string.h>
#include <SDL.h>

SDL_Rect srcRect = { 0,0,160,144 };
GB_COLOR palette[4]; // 由浅至深四种颜色

int video_init(GB_VIDEO* gpu) {
	SDL_InitSubSystem(SDL_INIT_VIDEO);
	gpu->surface = SDL_CreateRGBSurface(0, 160, 144, 32, 0xFF000000, 0xFF0000, 0xFF00, 0xFF);
	palette[0] = SDL_MapRGB(gpu->surface->format, 0xC7, 0xCA, 0xFF);
	palette[1] = SDL_MapRGB(gpu->surface->format, 0x68, 0x6F, 0xFF);
	palette[2] = SDL_MapRGB(gpu->surface->format, 0x68, 0x6F, 0xFF);
	palette[3] = SDL_MapRGB(gpu->surface->format, 0x00, 0x00, 0x20);
	gpu->window = SDL_CreateWindow("GameBoy Emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 320, 288, SDL_WINDOW_RESIZABLE);
	video_reset(gpu);
	return 0;
}

void video_free(GB_VIDEO* gpu) {
	SDL_FreeSurface(gpu->surface);
	SDL_DestroyWindow(gpu->window);
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

int video_reset(GB_VIDEO* gpu) {
	// 0xFF40 LCDC=0x91=0b10010001
	gpu->switchAll = 1;
	gpu->winMap = 0;
	gpu->switchWin = 0;
	gpu->tileSet = 1;
	gpu->bgMap = 0;
	gpu->objSize = 0;
	gpu->switchObj = 1;

	gpu->scy = 0;
	gpu->scx = 0;
	gpu->line = 0;
	gpu->lyc = 0;

	// 0xFF47 BGP = 0xFC
	gpu->bgPal[0] = palette[0]; // lightest
	gpu->bgPal[1] = palette[3];
	gpu->bgPal[2] = palette[3];
	gpu->bgPal[3] = palette[3]; // darkest

	gpu->objPals[0][0] = palette[3]; // lightest
	gpu->objPals[0][1] = palette[3];
	gpu->objPals[0][2] = palette[3];
	gpu->objPals[0][3] = palette[3]; // darkest

	gpu->objPals[1][0] = palette[3]; // lightest
	gpu->objPals[1][1] = palette[3];
	gpu->objPals[1][2] = palette[3];
	gpu->objPals[1][3] = palette[3]; // darkest

	gpu->winX = 0;
	gpu->winY = 0;

	gpu->mode = 1; // VBLANK
	gpu->modeClock = 0;

	memset(gpu->vram, 0, sizeof(gpu->vram));
	memset(gpu->oam, 0, sizeof(gpu->oam));
	memset(gpu->tileData, 0, sizeof(gpu->tileData));
	for (int i = 0; i < 40; i++)
	{
		gpu->objData[i].y = -16;
		gpu->objData[i].x = -8;
		gpu->objData[i].xflip = 0;
		gpu->objData[i].yflip = 0;
		gpu->objData[i].prio = 0;
		gpu->objData[i].tileNo = 0;
		gpu->objData[i].palNo = 0;
	}

	gpu->lcdIntEn = 0;
	return 0;
}

int video_renderLine(GB_VIDEO* gpu) {
	SDL_Surface* surface = gpu->surface;
	GB_COLOR* canvas = surface->pixels;
	if (gpu->switchAll) {
		if (gpu->switchBg) { // 绘制背景
			/* tile map: 32 x 32 x 1 字节 */
			int mapStart = (gpu->bgMap ? 0x1C00 : 0x1800);// VRAM offset for the tile map
			int mapY = ((gpu->line + gpu->scy) & 0xFF) >> 3; // Which line of tiles to use in the map, 8 行像素对应 1 行tile
			int mapX = (gpu->scx & 0xFF) >> 3; // Which tile to start with in the map line, 8 列像素对应 1 列tile
			int tileNo = gpu->vram[mapStart + 32 * mapY + mapX];// Read tile index from the background map
			if (gpu->tileSet == 0 && tileNo < 0x80) tileNo += 0x100;// tile set #0 修正索引

			/* tile 内部坐标 */
			int tileY = (gpu->line + gpu->scy) & 0b111;// Which line of pixels to use in the tiles
			int tileX = gpu->scx & 0b111;// Where in the tileline to start

			int canvasOffset = gpu->line * 160; // Where to render on the canvas
			for (int i = 0; i < 160; i++)
			{
				// 绘制一个点
				// SDL: 8bit: pixels内部存储surface->pixelFormat->palette内部索引
				//     >8bit: pixels内部存储颜色RGB
				int colorNo = gpu->tileData[tileNo][tileY][tileX];/*0-3*/
				canvas[canvasOffset + i] = gpu->bgPal[colorNo]; // 0x17000
				tileX++;
				if (tileX == 8)// 水平切换到下一个tile
				{
					tileX = 0;
					mapX = (mapX + 1) & 31; // 到边界折回, 位运算快
					tileNo = gpu->vram[mapStart + 32 * mapY + mapX];
					if (gpu->tileSet == 0 && tileNo < 0x80) tileNo += 256;
				}
			}
		} //if (gpu->switchBg) // 绘制背景

		if (gpu->switchObj) {
			for (int i = 0; i < 40; i++) { // 遍历40个spirit查找落在扫描线上的
				struct GB_OAM* obj = &gpu->objData[i];
				int objHeight = (gpu->objSize ? 16 : 8);
				if (obj->y - 16 <= gpu->line && (obj->y - 16 + objHeight) > gpu->line) // Check if this sprite falls on this scanline
				{
					//int tileY = gpu->line - obj->y + 16; // tileY>=8时读取到相连的下一个tile数据
					GB_BYTE* tileRow = gpu->tileData[obj->tileNo][(obj->yflip ? objHeight - 1 - gpu->line + obj->y - 16 : gpu->line - obj->y + 16)]; // 一行 tile 的数据
					int canvasOffset = gpu->line * surface->w + obj->x - 8; // Where to render on the canvas
					GB_COLOR* objPal = gpu->objPals[obj->palNo]; // Palette to use for this sprite
					for (int tileX = 0; tileX < 8; tileX++)
					{
						if ((obj->x - 8 + tileX) >= 0 && (obj->x - 8 + tileX) < 160) { // 在 160 x 144 屏幕渲染范围内(水平方向), AND
							int colorNo = tileRow[obj->xflip ? (7 - tileX) : tileX]; // 水平翻转
							if ((colorNo != 0) && (obj->prio == 0 || canvas[canvasOffset + tileX] == palette[0])) {
								// 颜色不为 0 (透明色), AND 精灵图在上层 OR (精灵图在下层)背景色透明 => 绘制
								canvas[canvasOffset + tileX] = objPal[colorNo];
							}
						}
					}
				}
			}// for(40)
		} //if (gpu->switchObj) // 绘制精灵图

		if (gpu->switchWin && gpu->line >= gpu->winY) {
			/* tile map: 32 x 32 x 1 字节 */
			int mapStart = (gpu->winMap ? 0x1C00 : 0x1800);// VRAM offset for the tile map
			int mapY = ((gpu->line - gpu->winY) & 0xFF) >> 3; // Which line of tiles to use in the map, 8 行像素对应 1 行tile
			int mapX = (gpu->winX & 0xFF) >> 3; // Which tile to start with in the map line, 8 列像素对应 1 列tile
			int tileNo = gpu->vram[mapStart + 32 * mapY + mapX];// Read tile index from the background map
			if (gpu->tileSet == 0 && tileNo < 0x80) tileNo += 0x100;// tile set #0 修正索引

			/* tile 内部坐标 */
			int tileY = (gpu->line - gpu->winY) & 0b111;// Which line of pixels to use in the tiles
			int tileX = gpu->winX & 0b111;// Where in the tileline to start

			int canvasOffset = gpu->line * 160; // Where to render on the canvas
			for (int i = 0; i < 160; i++)
			{
				// 绘制一个点
				// SDL: 8bit: pixels内部存储surface->pixelFormat->palette内部索引
				//     >8bit: pixels内部存储颜色RGB
				int colorNo = gpu->tileData[tileNo][tileY][tileX];/*0-3*/
				canvas[canvasOffset + i] = gpu->bgPal[colorNo]; // 0x17000
				tileX++;
				if (tileX == 8)// 水平切换到下一个tile
				{
					tileX = 0;
					mapX = (mapX + 1) & 31; // 到边界折回, 位运算快
					tileNo = gpu->vram[mapStart + 32 * mapY + mapX];
					if (gpu->tileSet == 0 && tileNo < 0x80) tileNo += 256;
				}
			}
		} //if (gpu->switchWin) // 绘制窗口
	}
	return 0;
}

int video_updateTile(GB_VIDEO* gpu, GB_WORD addr, GB_BYTE val) {
	addr &= 0x1FFE; // [偶数字节+奇数字节]=一行像素
	int tileNo = (addr & 0x1FF0) >> 4; // 每个tile占16个字节
	int tileY = (addr & 0b1110) >> 1; // 每行像素占2个字节
	int bitMask = 0b10000000;
	for (int tileX = 0; tileX < 8; tileX++)
	{
		gpu->tileData[tileNo][tileY][tileX] =
			((gpu->vram[addr + 1] & bitMask) ? 2 : 0) +
			((gpu->vram[addr] & bitMask) ? 1 : 0);// Update tile set
		bitMask >>= 1;
	}
	return 0;
}

int video_updateObj(GB_VIDEO* gpu, GB_WORD addr, GB_BYTE val) {
	addr &= 0x1FF;
	int objNo = addr >> 2;
	switch (addr & 0b11) // addr % 3
	{
	case 0:		gpu->objData[objNo].y = val; break;
	case 1:		gpu->objData[objNo].x = val; break;
	case 2:		gpu->objData[objNo].tileNo = val; break;
	default: // case 3
		gpu->objData[objNo].palNo = (val & 0x10 ? 1 : 0);
		gpu->objData[objNo].xflip = (val & 0x20 ? 1 : 0);
		gpu->objData[objNo].yflip = (val & 0x40 ? 1 : 0);
		gpu->objData[objNo].prio = (val & 0x80 ? 1 : 0);
		break;
	}
	return 0;
}

void vblankDelay() {
	static Uint32 lastFrameTicks = 0;
	static Uint32 last3FrameTicks = 0;
	static short frameCounter = 0;
	if (frameCounter == 2) { // 每三帧按50ms同步一次
		frameCounter = 0;
		while (SDL_GetTicks() - last3FrameTicks < 50) {
			SDL_Delay(1);
		}
		last3FrameTicks = SDL_GetTicks();
	}
	else { // 否则按16ms同步
		frameCounter++;
		while (SDL_GetTicks() - lastFrameTicks < 16) {
			SDL_Delay(1);
		}
	}
	lastFrameTicks = SDL_GetTicks();
}

int video_step(GB_VIDEO* gpu, int cycles) {
	/*
	 line    000000111111222222333333444444555...(144)....(153)00000
	 Mode 2  2_____2_____2_____2_____2_____2___________________2____
	 Mode 3  _33____33____33____33____33____33__________________3___
	 Mode 0  ___000___000___000___000___000___000________________000
	 Mode 1  ____________________________________11111111111111_____
	*/
	gpu->modeClock += cycles;
	switch (gpu->mode)
	{
	case 2: // Searching OAM
		if (gpu->modeClock >= 20)
		{
			gpu->modeClock -= 20;// Enter scanline mode 3
			gpu->mode = 3;
		}
		break;
	case 3: // Transferring Data to LCD Driver
		if (gpu->modeClock >= 43)
		{
			gpu->modeClock -= 43;
			video_renderLine(gpu); // Write a scanline to the framebuffer
			gpu->mode = 0;
			if (gpu->lcdIntEn & 0x08) gpu->parent->mem->intFlag |= 0b00000010; // LCDC 中断 Mode 0
		}
		break;
	case 0: // 水平复位 HBLANK (line = 0-144)
		if (gpu->modeClock >= 51) // 每51个机器周期 水平复位一次
		{
			gpu->modeClock -= 51;
			gpu->line++;  // 下移一行
			if (gpu->line == 144) { // Enter hblank
				gpu->mode = 1;
				if (gpu->lcdIntEn & 0x10) gpu->parent->mem->intFlag |= 0b00000010; // LCDC 中断 Mode 1
				if (gpu->switchAll) {
					gpu->parent->mem->intFlag |= 0b00000001; // VBLANK 中断
					SDL_Surface* surface = SDL_GetWindowSurface(gpu->window);
					SDL_BlitScaled(gpu->surface, &srcRect, surface, &surface->clip_rect);
					SDL_UpdateWindowSurface(gpu->window); // 刷新一帧
					//printf("Fresh at %d\n", SDL_GetTicks());
					vblankDelay();
					//printf("After delay at %d\n", SDL_GetTicks());
				}
			}
			else {
				gpu->mode = 2;
				if (gpu->lcdIntEn & 0x20) gpu->parent->mem->intFlag |= 0b00000010; // LCDC 中断 Mode 2
			}
		}
		break;
	case 1: // 垂直复位 VBLANK (456 x 10 lines, line = 144-154)
		if (gpu->modeClock >= 114)
		{
			gpu->modeClock -= 114;
			gpu->line++;
			if (gpu->line == 154)
			{
				gpu->line = 0;
				gpu->mode = 2;
				if (gpu->lcdIntEn & 0x20) gpu->parent->mem->intFlag |= 0b00000010; // LCDC 中断 Mode 2
			}
		}
		break;
	}
	if (gpu->switchAll && (gpu->lcdIntEn & 0x40) && gpu->line == gpu->lyc) {
		gpu->parent->mem->intFlag |= 0b00000010; // LCDC 中断 LYC=LY
	}
	return 0;
}

GB_BYTE video_readByte(GB_VIDEO* gpu, GB_WORD addr) {
	switch (addr)
	{
	case 0xFF40: // LCD Control
		return (gpu->switchBg ? 0b00000001 : 0) |
			(gpu->switchObj ? 0b00000010 : 0) |
			(gpu->objSize ? 0b00000100 : 0) |
			(gpu->bgMap ? 0b00001000 : 0) |
			(gpu->tileSet ? 0b00010000 : 0) |
			(gpu->switchWin ? 0b00100000 : 0) |
			(gpu->winMap ? 0b01000000 : 0) |
			(gpu->switchAll ? 0b10000000 : 0);
	case 0xFF41: // STAT
		return (gpu->mode) |
			(gpu->line == gpu->lyc ? 0b00000100 : 0) |
			(gpu->lcdIntEn << 4);
	case 0xFF42: // Scroll Y
		return gpu->scy;
	case 0xFF43: // Scroll X
		return gpu->scx;
	case 0xFF44: // Current scanline
		return gpu->line;
	case 0xFF45: // Line compare
		return gpu->lyc;
	case 0xFF47: // BGP
		return ((gpu->bgPal[0] << 0) ||
			(gpu->bgPal[1] << 2) ||
			(gpu->bgPal[2] << 4) ||
			(gpu->bgPal[3] << 6));
	case 0xFF48: // BGP #0
	case 0xFF49: { // BGP #1
		int palNo = addr & 1;
		return ((gpu->objPals[palNo][0] << 0) ||
			(gpu->objPals[palNo][1] << 2) ||
			(gpu->objPals[palNo][2] << 4) ||
			(gpu->objPals[palNo][3] << 6)); }
	case 0xFF4A:
		return gpu->winY;
	case 0xFF4B:
		return gpu->winX + 7;
	case 0xFF46: // OAM DMA
	default:
		return 0;
	}
}

void video_writeByte(GB_VIDEO* gpu, GB_WORD addr, GB_BYTE val) {
	switch (addr)
	{
	case 0xFF40:// LCD Control
		gpu->switchBg = ((val & 0b00000001) ? 1 : 0);
		gpu->switchObj = ((val & 0b00000010) ? 1 : 0);
		gpu->objSize = ((val & 0b00000100) ? 1 : 0);
		gpu->bgMap = ((val & 0b00001000) ? 1 : 0);
		gpu->tileSet = ((val & 0b00010000) ? 1 : 0);
		gpu->switchWin = ((val & 0b00100000) ? 1 : 0);
		gpu->winMap = ((val & 0b01000000) ? 1 : 0);
		gpu->switchAll = ((val & 0b10000000) ? 1 : 0);
		break;
	case 0xFF41: // STAT
		gpu->lcdIntEn = (val & 0xF0);
		break;
	case 0xFF42: // Scroll Y
		gpu->scy = val;
		break;
	case 0xFF43: // Scroll X
		gpu->scx = val;
		break;
	case 0xFF45: // Line compare
		gpu->lyc = val;
		break;
	case 0xFF46: // OAM DMA
		for (int i = 0; i < 160; i++)
		{
			GB_BYTE dma = readByte(gpu->parent->mem, (val << 8) + i);
			gpu->oam[i] = dma;
			video_updateObj(gpu, 0xFE00 + i, dma);
		}
		break;
	case 0xFF47: // Background palette
		for (int i = 0; i < 4; i++)
		{
			switch (val & 0b11)
			{
			case 0: gpu->bgPal[i] = palette[0]; break;
			case 1: gpu->bgPal[i] = palette[1]; break;
			case 2: gpu->bgPal[i] = palette[2]; break;
			case 3: gpu->bgPal[i] = palette[3]; break;
			}
			val >>= 2;
		}
		break;
	case 0xFF48: // Object palettes #0
	case 0xFF49: // #1
		for (int i = 0, palNo = addr & 1; i < 4; i++)
		{
			switch (val & 0b11)
			{
			case 0: gpu->objPals[palNo][i] = palette[0]; break;
			case 1: gpu->objPals[palNo][i] = palette[1]; break;
			case 2: gpu->objPals[palNo][i] = palette[2]; break;
			case 3: gpu->objPals[palNo][i] = palette[3]; break;
			}
			val >>= 2;
		}
		break;
	case 0xFF4A:
		gpu->winY = val;
		break;
	case 0xFF4B:
		gpu->winX = val - 7;
		break;
	case 0xFF44: // Scan Line
	default:
		break;
	}
}
