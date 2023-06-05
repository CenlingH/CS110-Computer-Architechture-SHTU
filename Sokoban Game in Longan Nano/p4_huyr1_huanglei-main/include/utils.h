#ifndef __UTILS_H
#define __UTILS_H

#define BOARD_VER 1 /*fill in your board version here. i.e. 0 or 1*/

#if BOARD_VER < 0 || BOARD_VER > 1
#error INVALID_BOARD_VER
#endif

#if BOARD_VER == 0

#define JOY_LEFT GPIO_PIN_0
#define JOY_DOWN GPIO_PIN_1
#define JOY_RIGHT GPIO_PIN_2
#define JOY_UP GPIO_PIN_5
#define JOY_CTR GPIO_PIN_4

#define BUTTON_1 GPIO_PIN_6
#define BUTTON_2 GPIO_PIN_7

#elif BOARD_VER == 1

#define JOY_LEFT GPIO_PIN_13
#define JOY_DOWN GPIO_PIN_0
#define JOY_RIGHT GPIO_PIN_1
#define JOY_UP GPIO_PIN_5
#define JOY_CTR GPIO_PIN_4

#define BUTTON_1 GPIO_PIN_6
#define BUTTON_2 GPIO_PIN_7

#endif


enum ObjectType{
        player,
        box,
        wall,
        goal,
        boxOnGoal,
        playerOnGoal,
        empty,
        error
};

typedef struct Object {
        short logincalX; //logical x
        short logincalY; //logical y
        short physicalX; //physical x
        short physicalY; //physical y
        short physicalWidth; //physical width
        short physicalHeight; //physical height
        short renderZ; //render z, higher means on top. 
        enum ObjectType type; //type of object
} Object;

typedef struct GameConfig {
        short level;
        short objectCount;
        short playerIndex;
        Object *objects;
} GameConfig;

int Get_Button(int ch);
int Get_BOOT0(void);
Object createPlayer(short logicX,short logicY);
Object createWall(short logicX,short logicY);
Object createBox(short logicX,short logicY);
Object createGoal(short logicX,short logicY);
Object createEmpty(short logicX,short logicY);
Object createObject(short logincalX, short logincalY, short physicalX, short physicalY, short physicalWidth, short physicalHeight, short renderZ, enum ObjectType type);
void renderObject(Object obj);
void debug(const char* msg);
void debugf(const char *format, ...);
void slowDebugf(const char *format, ...);
#endif