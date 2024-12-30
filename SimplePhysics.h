#ifndef _SIMPLE_PHYSICS_H
#define _SIMPLE_PHYSICS_H

#include <time.h>



#ifdef SimplePhysics__PERFORMANCE
typedef float _SimplePhysics__float;
#else
typedef double _SimplePhysics__float;
#endif

#define SimplePhysics__MAX_OBJECTS_COUNT 64



typedef enum {
        SimplePhysics__SUCCESS,
        SimplePhysics__ERROR_REACHED_MAX_OBJECTS_CAP,
        SimplePhysics__ERROR_FAILED_TO_GET_TIME
} SimplePhysics__Error;

typedef struct {
        _SimplePhysics__float x;
        _SimplePhysics__float y;
        _SimplePhysics__float z;
} SimplePhysics__Vector3;

typedef struct {
        _SimplePhysics__float xLeft;
        _SimplePhysics__float xRight;
        _SimplePhysics__float yBottom;
        _SimplePhysics__float yTop;
        _SimplePhysics__float zNear;
        _SimplePhysics__float zFar;
} SimplePhysics__CollisionBox;

typedef struct {

        void *extraData; // Optional
        SimplePhysics__CollisionBox *collisionBox;
        SimplePhysics__Vector3 *position;
        SimplePhysics__Vector3 *velocity;
        int bStatic;

} SimplePhysics__Object;

typedef void (*SimplePhysics__CollisionCallback)(SimplePhysics__Object* collider, SimplePhysics__Object* collidee);



int _SimplePhysics__bInitialized = 0;

struct timespec _SimplePhysics__previousTime;

SimplePhysics__Object _SimplePhysics__objects[SimplePhysics__MAX_OBJECTS_COUNT];
int _SimplePhysics__objects_count = 0;



SimplePhysics__Error SimplePhysics__Add(
        SimplePhysics__CollisionBox *collisionBox,
        int bStatic,
        SimplePhysics__Vector3 *position,
        SimplePhysics__Vector3 *velocity,
        void *extraData // Optional
        ) {

        if (_SimplePhysics__objects_count+1 == SimplePhysics__MAX_OBJECTS_COUNT) return SimplePhysics__ERROR_REACHED_MAX_OBJECTS_CAP;

        _SimplePhysics__objects[_SimplePhysics__objects_count].collisionBox = collisionBox;
        _SimplePhysics__objects[_SimplePhysics__objects_count].position = position;
        _SimplePhysics__objects[_SimplePhysics__objects_count].velocity = velocity;
        _SimplePhysics__objects[_SimplePhysics__objects_count].bStatic = bStatic;
        _SimplePhysics__objects[_SimplePhysics__objects_count].extraData = extraData;

        _SimplePhysics__objects_count++;

        return SimplePhysics__SUCCESS;

}



SimplePhysics__Error SimplePhysics__Tick(_SimplePhysics__float gravity, SimplePhysics__CollisionCallback collisionCallback) {

        if (!_SimplePhysics__bInitialized) {
                if (clock_gettime(CLOCK_MONOTONIC, &_SimplePhysics__previousTime) != 0) return SimplePhysics__ERROR_FAILED_TO_GET_TIME;
                _SimplePhysics__bInitialized = 1;
        }



        // Get delta time & update saved time for next tick

        struct timespec currentTime;
        if (clock_gettime(CLOCK_MONOTONIC, &currentTime) != 0) return SimplePhysics__ERROR_FAILED_TO_GET_TIME;

        _SimplePhysics__float deltaTime = (_SimplePhysics__float) (
                ((currentTime.tv_sec * 1000000000) + currentTime.tv_nsec) -
                ((_SimplePhysics__previousTime.tv_sec * 1000000000) + _SimplePhysics__previousTime.tv_nsec)
                ) / 1000000000
        ;

        _SimplePhysics__previousTime.tv_sec = currentTime.tv_sec;
        _SimplePhysics__previousTime.tv_nsec = currentTime.tv_nsec;



        // Simulate physics for non-static objects

        for (int i = 0; i < _SimplePhysics__objects_count; i++) {
                if (_SimplePhysics__objects[i].bStatic) break;

                // Apply gravity (*unless already falling at or above terminal velocity)
                if (_SimplePhysics__objects[i].velocity->y < gravity) {
                        _SimplePhysics__objects[i].velocity->y -= gravity;
                }



                // Apply velocity

                _SimplePhysics__objects[i].position->x += _SimplePhysics__objects[i].velocity->x * deltaTime;
                _SimplePhysics__objects[i].position->y += _SimplePhysics__objects[i].velocity->y * deltaTime;
                _SimplePhysics__objects[i].position->z += _SimplePhysics__objects[i].velocity->z * deltaTime;



                // Check for & resolve collisions

                for (int i2 = 0; i2 < _SimplePhysics__objects_count; i2++) {

                        if (i2 == i) continue;

                        if (
                                // x1left < x2right
                                (_SimplePhysics__objects[i].position->x - _SimplePhysics__objects[i].collisionBox->xLeft)
                                <
                                (_SimplePhysics__objects[i2].position->x + _SimplePhysics__objects[i2].collisionBox->xRight)

                                &&

                                // x1right > x2left
                                (_SimplePhysics__objects[i].position->x + _SimplePhysics__objects[i].collisionBox->xRight)
                                >
                                (_SimplePhysics__objects[i2].position->x - _SimplePhysics__objects[i2].collisionBox->xLeft)

                                &&

                                // y1bottom < y2top
                                (_SimplePhysics__objects[i].position->y - _SimplePhysics__objects[i].collisionBox->yBottom)
                                <
                                (_SimplePhysics__objects[i2].position->y + _SimplePhysics__objects[i2].collisionBox->yTop)

                                &&

                                // y1top > y2bottom
                                (_SimplePhysics__objects[i].position->y + _SimplePhysics__objects[i].collisionBox->yTop)
                                >
                                (_SimplePhysics__objects[i2].position->y - _SimplePhysics__objects[i2].collisionBox->yBottom)

                                &&

                                // z1near < z2far
                                (_SimplePhysics__objects[i].position->z - _SimplePhysics__objects[i].collisionBox->zNear)
                                <
                                (_SimplePhysics__objects[i2].position->z + _SimplePhysics__objects[i2].collisionBox->zFar)

                                &&

                                // z1far > z2near
                                (_SimplePhysics__objects[i].position->z + _SimplePhysics__objects[i].collisionBox->zFar)
                                >
                                (_SimplePhysics__objects[i2].position->z - _SimplePhysics__objects[i2].collisionBox->zNear)
                        ) {

                                if (collisionCallback != 0) collisionCallback(&_SimplePhysics__objects[i], &_SimplePhysics__objects[i2]);

                                // Resolve collision (push to nearest side)

                                _SimplePhysics__float xLeftOverlap = // x2right - x1left
                                        (_SimplePhysics__objects[i2].position->x + _SimplePhysics__objects[i2].collisionBox->xRight)
                                        -
                                        (_SimplePhysics__objects[i].position->x - _SimplePhysics__objects[i].collisionBox->xLeft)
                                ;
                                _SimplePhysics__float xRightOverlap = // x1right - x2left
                                        (_SimplePhysics__objects[i].position->x + _SimplePhysics__objects[i].collisionBox->xRight)
                                        -
                                        (_SimplePhysics__objects[i2].position->x - _SimplePhysics__objects[i2].collisionBox->xLeft)
                                ;
                                _SimplePhysics__float xOverlap = (xLeftOverlap < xRightOverlap) ? xLeftOverlap : xRightOverlap;

                                _SimplePhysics__float yBottomOverlap = // y2top - y1bottom
                                        (_SimplePhysics__objects[i2].position->y + _SimplePhysics__objects[i2].collisionBox->yTop)
                                        -
                                        (_SimplePhysics__objects[i].position->y - _SimplePhysics__objects[i].collisionBox->yBottom)
                                ;
                                _SimplePhysics__float yTopOverlap = // y1top - y2bottom
                                        (_SimplePhysics__objects[i].position->y + _SimplePhysics__objects[i].collisionBox->yTop)
                                        -
                                        (_SimplePhysics__objects[i2].position->y - _SimplePhysics__objects[i2].collisionBox->yBottom)
                                ;
                                _SimplePhysics__float yOverlap = (yBottomOverlap < yTopOverlap) ? yBottomOverlap : yTopOverlap;

                                _SimplePhysics__float zNearOverlap = // z2far - z1near
                                        (_SimplePhysics__objects[i2].position->z + _SimplePhysics__objects[i2].collisionBox->zFar)
                                        -
                                        (_SimplePhysics__objects[i].position->z - _SimplePhysics__objects[i].collisionBox->zNear)
                                ;
                                _SimplePhysics__float zFarOverlap = // z1far - z2near
                                        (_SimplePhysics__objects[i].position->z + _SimplePhysics__objects[i].collisionBox->zFar)
                                        -
                                        (_SimplePhysics__objects[i2].position->z - _SimplePhysics__objects[i2].collisionBox->zNear)
                                ;
                                _SimplePhysics__float zOverlap = (zNearOverlap < zFarOverlap) ? zNearOverlap : zFarOverlap;

                                if (xOverlap < yOverlap && xOverlap < zOverlap) {

                                        if (xLeftOverlap < xRightOverlap) _SimplePhysics__objects[i].position->x = // Push to right side of collidee's collision box
                                                (_SimplePhysics__objects[i2].position->x + _SimplePhysics__objects[i2].collisionBox->xRight)
                                                + _SimplePhysics__objects[i].collisionBox->xLeft;

                                        else _SimplePhysics__objects[i].position->x = // Push to left side of collidee's collision box
                                                (_SimplePhysics__objects[i2].position->x - _SimplePhysics__objects[i2].collisionBox->xLeft)
                                                - _SimplePhysics__objects[i].collisionBox->xRight;

                                }

                                else if (yOverlap < zOverlap) {

                                        if (yBottomOverlap < yTopOverlap) _SimplePhysics__objects[i].position->y = // Push to top side of collidee's collision box
                                                (_SimplePhysics__objects[i2].position->y + _SimplePhysics__objects[i2].collisionBox->yTop)
                                                + _SimplePhysics__objects[i].collisionBox->yBottom;

                                        else _SimplePhysics__objects[i].position->y = // Push to bottom side of collidee's collision box
                                                (_SimplePhysics__objects[i2].position->y - _SimplePhysics__objects[i2].collisionBox->yBottom)
                                                - _SimplePhysics__objects[i].collisionBox->yTop;

                                }

                                else {

                                        if (zNearOverlap < zFarOverlap) _SimplePhysics__objects[i].position->z = // Push to far side of collidee's collision box
                                                (_SimplePhysics__objects[i2].position->z + _SimplePhysics__objects[i2].collisionBox->zFar)
                                                + _SimplePhysics__objects[i].collisionBox->zNear;

                                        else _SimplePhysics__objects[i].position->z = // Push to near side of collidee's collision box
                                                (_SimplePhysics__objects[i2].position->z - _SimplePhysics__objects[i2].collisionBox->zNear)
                                                - _SimplePhysics__objects[i].collisionBox->zFar;

                                }

                        }

                }

        }



        // Reset for next tick;
        // So that library user can change contents of
        // physics world by calling or not calling
        // "SimplePhysics__Add()" between ticks
        _SimplePhysics__objects_count = 0;



        return SimplePhysics__SUCCESS;

}



#endif // _SIMPLE_PHYSICS_H