//Through lexical and syntatic analysis

const int N = 2;
int global_arr[2][2] = {{1, 2}, {3, 4}};

// 测试数组传参
void array_param_func(int arr[][2]) {
    arr[0][0] = arr[0][0] * 2;
    arr[0][1] = arr[0][1] * 2;
    arr[1][0] = arr[1][0] * 2;
    arr[1][1] = arr[1][1] * 2;
}

// 测试部分数组传参
void partial_array_param_func(int arr[]) {
    arr[0] = arr[0] + 1;
}

int main() {
    int local_arr[2][2] = {{5, 6}, {7, 8}};
    int sum;

    // 测试数组元素使用
    sum = local_arr[0][0] + local_arr[1][1];
    printf("%d\n", sum);

    // 测试数组传参
    array_param_func(local_arr);
    printf("%d %d\n", local_arr[0][0], local_arr[0][1]);
    printf("%d %d\n", local_arr[1][0], local_arr[1][1]);

    // 测试部分数组传参
    partial_array_param_func(local_arr[1]);
    printf("%d\n", local_arr[1][0]);

    // 测试全局数组元素使用
    sum = global_arr[0][0] + global_arr[1][1];
    printf("%d\n", sum);

    return 0;
}    