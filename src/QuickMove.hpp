#pragma once

// Namespace with useful enum classes
namespace quickmove {
    // Type of transform
    enum class TransformType {
        Move = 1,
        Rotate = 2,
        Flip = 3,
    };

    // Distance in which the buttons will move the objects on the grid
    enum class MoveSize {
        Tiny = 0,
        Small = 1,
        Half = 2,
        Normal = 3,
        Big = 4
    };

    // Direction in which objects will be moved on the grid
    enum class MoveDirection {
        Up = 1,
        Down = 2,
        Left = 3,
        Right = 4
    };
};