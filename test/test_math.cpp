#define CGLTF_IMPLEMENTATION
#include "../cgltf.h"

// Performs matrix-vector multiplication, as in (4x4) * (4x1) = (4x1)
static void transform(const cgltf_float matrix[16], const cgltf_float source[4], cgltf_float target[4]) {
    target[0] = matrix[0] * source[0] + matrix[4] * source[1] + matrix[ 8] * source[2] + matrix[12] * source[3];
    target[1] = matrix[1] * source[0] + matrix[5] * source[1] + matrix[ 9] * source[2] + matrix[13] * source[3];
    target[2] = matrix[2] * source[0] + matrix[6] * source[1] + matrix[10] * source[2] + matrix[14] * source[3];
    target[3] = matrix[3] * source[0] + matrix[7] * source[1] + matrix[11] * source[2] + matrix[15] * source[3];
}

static void set(cgltf_float target[3], float x, float y, float z) {
    target[0] = x;
    target[1] = y;
    target[2] = z;
}

static void check(cgltf_float target[3], float x, float y, float z) {
    if (target[0] != x || target[1] != y || target[2] != z) {
        fprintf(stderr, "Mismatch detected.\n");
        exit(1);
    }
}

static void check_matrix(const cgltf_float target[16], const cgltf_float expected[16]) {
    for (int i = 0; i < 16; ++i) {
        if (target[i] != expected[i]) {
            fprintf(stderr, "Matrix mismatch at %d: got %f, expected %f.\n", i, target[i], expected[i]);
            exit(1);
        }
    }
}

int main(int, char**)
{
    cgltf_node node = {};

    cgltf_float matrix[16];
    cgltf_float source[4] = {1, 2, 3, 1};
    cgltf_float target[4];

    set(node.scale, 1, 1, 1);
    set(node.translation, 1, 0, 0);
    cgltf_node_transform_local(&node, matrix);
    transform(matrix, source, target);
    check(target, 2, 2, 3);

    set(node.scale, 3, 1, 1);
    set(node.translation, 0, 0, 0);
    cgltf_node_transform_local(&node, matrix);
    transform(matrix, source, target);
    check(target, 3, 2, 3);

    set(node.scale, 1, 3, 1);
    set(node.translation, 1, 0, 0);
    cgltf_node_transform_local(&node, matrix);
    transform(matrix, source, target);
    check(target, 2, 6, 3);

    // G * P * L, where G is a full 4x4 matrix, P is a matrix with a
    // projective fourth row, and L is a regular TRS translation.
    // G = 2 * I
    // P = [1 0 0 1; 0 1 0 2; 0 0 1 3; 0 0 0 2]
    // L = [1 0 0 4; 0 1 0 5; 0 0 1 6; 0 0 0 1]
    // Hand-computed G * P * L:
    // [2 0 0 10; 0 2 0 14; 0 0 2 18; 0 0 0 4]
    cgltf_node grandparent = {};
    grandparent.has_matrix = 1;
    cgltf_float grandparent_matrix[16] = {
        2, 0, 0, 0,
        0, 2, 0, 0,
        0, 0, 2, 0,
        0, 0, 0, 2,
    };
    memcpy(grandparent.matrix, grandparent_matrix, sizeof(grandparent_matrix));

    cgltf_node parent = {};
    parent.parent = &grandparent;
    parent.has_matrix = 1;
    cgltf_float parent_matrix[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        1, 2, 3, 2,
    };
    memcpy(parent.matrix, parent_matrix, sizeof(parent_matrix));

    cgltf_node child = {};
    child.parent = &parent;
    set(child.translation, 4, 5, 6);
    set(child.scale, 1, 1, 1);
    child.rotation[3] = 1;

    cgltf_float expected_world[16] = {
        2, 0, 0, 0,
        0, 2, 0, 0,
        0, 0, 2, 0,
        10, 14, 18, 4,
    };
    cgltf_float world[16];
    cgltf_node_transform_world(&child, world);
    check_matrix(world, expected_world);

    return 0;
}
