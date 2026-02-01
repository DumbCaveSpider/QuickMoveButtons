#pragma once

// Namespace with useful enum classes
namespace quickmove {
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

    // Type of transform
    enum class TransformType {
        Move = 1,
        Rotate = 2,
        Flip = 3,
    };

    // Returns the EditCommand based on object move size and direction
    EditCommand getEditCmd(MoveSize moveSize, MoveDirection moveDir) {
        switch (moveSize) {
        case MoveSize::Tiny:
            switch (moveDir) {
            case MoveDirection::Up:    return EditCommand::TinyUp;
            case MoveDirection::Down:  return EditCommand::TinyDown;
            case MoveDirection::Left:  return EditCommand::TinyLeft;
            case MoveDirection::Right: return EditCommand::TinyRight;
            };
            break;

        case MoveSize::Small:
            switch (moveDir) {
            case MoveDirection::Up:    return EditCommand::SmallUp;
            case MoveDirection::Down:  return EditCommand::SmallDown;
            case MoveDirection::Left:  return EditCommand::SmallLeft;
            case MoveDirection::Right: return EditCommand::SmallRight;
            };
            break;

        case MoveSize::Half:
            switch (moveDir) {
            case MoveDirection::Up:    return EditCommand::HalfUp;
            case MoveDirection::Down:  return EditCommand::HalfDown;
            case MoveDirection::Left:  return EditCommand::HalfLeft;
            case MoveDirection::Right: return EditCommand::HalfRight;
            };
            break;

        case MoveSize::Normal:
            switch (moveDir) {
            case MoveDirection::Up:    return EditCommand::Up;
            case MoveDirection::Down:  return EditCommand::Down;
            case MoveDirection::Left:  return EditCommand::Left;
            case MoveDirection::Right: return EditCommand::Right;
            };
            break;

        case MoveSize::Big:
            switch (moveDir) {
            case MoveDirection::Up:    return EditCommand::BigUp;
            case MoveDirection::Down:  return EditCommand::BigDown;
            case MoveDirection::Left:  return EditCommand::BigLeft;
            case MoveDirection::Right: return EditCommand::BigRight;
            };
            break;

        default:
            return EditCommand::SmallUp;
        };

        return EditCommand::SmallUp;
    };

    // Get the string name for a move size for debugging
    std::string getMoveSizeName(MoveSize moveSize) {
        switch (moveSize) {
        case MoveSize::Tiny:
            return "Tiny";

        case MoveSize::Small:
            return "Small";

        case MoveSize::Half:
            return "Half";

        case MoveSize::Normal:
            return "Normal";

        case MoveSize::Big:
            return "Big";

        default:
            return "Small";
        };
    };

    // Get the sprite name and scale for arrow icons based on move size
    std::pair<std::string, float> getMoveSizeIconInfo(MoveSize moveSize) {
        float defScale = 0.875f;

        switch (moveSize) {
        case MoveSize::Tiny:
            return { "edit_upBtn_001.png", 0.625f };

        case MoveSize::Small:
            return { "edit_upBtn_001.png", defScale };

        case MoveSize::Half:
            return { "edit_upBtn5_001.png", defScale };

        case MoveSize::Normal:
            return { "edit_upBtn2_001.png", defScale };

        case MoveSize::Big:
            return { "edit_upBtn3_001.png", defScale };

        default:
            return { "edit_upBtn_001.png", defScale };
        };
    };
};