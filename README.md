Simple single-header cross-platform 3D AABB physics simulation library

# Usage:
1) Add objects with `SimplePhysics__Add()`
2) Call `SimplePhysics__Tick()`

# Example:
(press ENTER to do a physics tick (scaled by deltatime))
```c
#include <stdio.h>

#include "SimplePhysics.h"



void CollisionCallback(SimplePhysics__Object *collider, SimplePhysics__Object *collidee) {

        printf("%s hit %s!\n", collider->extraData, collidee->extraData);

}



int main() {

        SimplePhysics__Vector3 fallingObjectPosition = {0, 30, 0};
        SimplePhysics__Vector3 fallingObjectVelocity = {0, 0, 0};

        SimplePhysics__Vector3 staticObjectPosition = {0, 0, 0};



        while (getchar() == '\n') {

                SimplePhysics__Add(
                        &(SimplePhysics__CollisionBox) {
                                .xLeft = 10,
                                .xRight = 10,
                                .yBottom = 10,
                                .yTop = 10,
                                .zNear = 10,
                                .zFar = 10
                        },
                        0,
                        &fallingObjectPosition,
                        &fallingObjectVelocity,
                        "Falling object"
                );

                SimplePhysics__Add(
                        &(SimplePhysics__CollisionBox) {
                                .xLeft = 10,
                                .xRight = 10,
                                .yBottom = 10,
                                .yTop = 10,
                                .zNear = 10,
                                .zFar = 10
                        },
                        1,
                        &staticObjectPosition,
                        0,
                        "Static object"
                );



                SimplePhysics__Tick(1, &CollisionCallback);



                printf(
                        "Falling object position:\n%f\n%f\n%f",
                        fallingObjectPosition.x,
                        fallingObjectPosition.y,
                        fallingObjectPosition.z
                );
                printf(
                        "Falling object velocity:\n%f\n%f\n%f\n",
                        fallingObjectVelocity.x,
                        fallingObjectVelocity.y,
                        fallingObjectVelocity.z
                );

        }



        return 0;

}
```
