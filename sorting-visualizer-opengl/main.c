#include <GL/glut.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

#define SIZE 50
#define DELAY 200

int array[SIZE];
bool sorting = false, paused = false;
int i = 0, j = 0, algorithm = 0;
char algorithmName[50];
char complexityText[100];

int mergeCurrSize = 1;
int quickStack[SIZE], top = -1;
int heapSize = SIZE;
int heapPhase = 0;

enum {
    BUBBLE_SORT = 1,
    SELECTION_SORT,
    INSERTION_SORT,
    MERGE_SORT,
    QUICK_SORT,
    HEAP_SORT
};

void initArray() {
    srand(time(0));
    for (int i = 0; i < SIZE; i++) {
        array[i] = rand() % 100 + 1;
    }

    // Reset state
    i = j = 0;
    sorting = false;
    paused = false;
    top = -1;
    mergeCurrSize = 1;
    heapSize = SIZE;
    heapPhase = 0;
    algorithm = 0;
    strcpy(algorithmName, "");
    strcpy(complexityText, "");
}

void displayText(float x, float y, const char *text) {
    glRasterPos2f(x, y);
    for (int k = 0; k < strlen(text); k++) {
        glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, text[k]);
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Display algorithm name
    glColor3f(1.0, 1.0, 1.0);
    displayText(-0.9, 0.9, algorithmName);

    // Display complexity
    glColor3f(0.8, 0.8, 0.0);
    displayText(-0.9, 0.8, complexityText);

    // Draw bars
    for (int k = 0; k < SIZE; k++) {
        if (k == j || k == j + 1 || k == i)
            glColor3f(1.0, 0.0, 0.0);
        else
            glColor3f(0.0, 1.0, 0.0);

        float x = -1.0 + 2.0 * k / SIZE;
        float y = -1.0 + 2.0 * array[k] / 100;
        glBegin(GL_QUADS);
        glVertex2f(x, -1);
        glVertex2f(x + 1.8 / SIZE, -1);
        glVertex2f(x + 1.8 / SIZE, y);
        glVertex2f(x, y);
        glEnd();
    }

    glutSwapBuffers();
}

void bubbleSortStep() {
    if (i < SIZE - 1) {
        if (j < SIZE - i - 1) {
            if (array[j] > array[j + 1]) {
                int temp = array[j];
                array[j] = array[j + 1];
                array[j + 1] = temp;
            }
            j++;
        } else {
            j = 0;
            i++;
        }
    } else sorting = false;
}

void selectionSortStep() {
    static int min_idx = 0;
    if (i < SIZE - 1) {
        if (j < SIZE) {
            if (array[j] < array[min_idx])
                min_idx = j;
            j++;
        } else {
            int temp = array[min_idx];
            array[min_idx] = array[i];
            array[i] = temp;
            i++;
            j = i + 1;
            min_idx = i;
        }
    } else sorting = false;
}

void insertionSortStep() {
    static int key, k;
    if (i < SIZE) {
        if (j == 0) {
            key = array[i];
            k = i - 1;
            j = 1;
        }
        if (k >= 0 && array[k] > key) {
            array[k + 1] = array[k];
            k--;
        } else {
            array[k + 1] = key;
            i++;
            j = 0;
        }
    } else sorting = false;
}

void merge(int l, int m, int r) {
    int n1 = m - l + 1, n2 = r - m;
    int L[n1], R[n2];

    for (int x = 0; x < n1; x++) L[x] = array[l + x];
    for (int y = 0; y < n2; y++) R[y] = array[m + 1 + y];

    int i = 0, j = 0, k = l;

    while (i < n1 && j < n2) array[k++] = (L[i] <= R[j]) ? L[i++] : R[j++];
    while (i < n1) array[k++] = L[i++];
    while (j < n2) array[k++] = R[j++];
}

void mergeSortStep() {
    if (mergeCurrSize >= SIZE) {
        sorting = false;
        return;
    }

    for (int left_start = 0; left_start < SIZE - 1; left_start += 2 * mergeCurrSize) {
        int mid = left_start + mergeCurrSize - 1;
        int right_end = (left_start + 2 * mergeCurrSize - 1 < SIZE) ? (left_start + 2 * mergeCurrSize - 1) : (SIZE - 1);
        if (mid < right_end)
            merge(left_start, mid, right_end);
    }

    mergeCurrSize *= 2;
}

int partition(int low, int high) {
    int pivot = array[high], i = (low - 1);
    for (int j = low; j < high; j++) {
        if (array[j] <= pivot) {
            i++;
            int temp = array[i]; array[i] = array[j]; array[j] = temp;
        }
    }
    int temp = array[i + 1]; array[i + 1] = array[high]; array[high] = temp;
    return (i + 1);
}

void quickSortStep() {
    if (top == -1) {
        quickStack[++top] = 0;
        quickStack[++top] = SIZE - 1;
    }

    if (top >= 0) {
        int high = quickStack[top--];
        int low = quickStack[top--];

        int p = partition(low, high);
        if (p - 1 > low) {
            quickStack[++top] = low;
            quickStack[++top] = p - 1;
        }
        if (p + 1 < high) {
            quickStack[++top] = p + 1;
            quickStack[++top] = high;
        }
    } else sorting = false;
}

void heapify(int n, int i) {
    int largest = i, l = 2 * i + 1, r = 2 * i + 2;
    if (l < n && array[l] > array[largest]) largest = l;
    if (r < n && array[r] > array[largest]) largest = r;
    if (largest != i) {
        int temp = array[i]; array[i] = array[largest]; array[largest] = temp;
        heapify(n, largest);
    }
}

void heapSortStep() {
    if (heapPhase == 0) {
        for (int i = heapSize / 2 - 1; i >= 0; i--)
            heapify(heapSize, i);
        heapPhase = 1;
    } else if (heapSize > 0) {
        int temp = array[0];
        array[0] = array[heapSize - 1];
        array[heapSize - 1] = temp;
        heapify(--heapSize, 0);
    } else sorting = false;
}

void timer(int value) {
    if (sorting && !paused) {
        switch (algorithm) {
        case BUBBLE_SORT: bubbleSortStep(); break;
        case SELECTION_SORT: selectionSortStep(); break;
        case INSERTION_SORT: insertionSortStep(); break;
        case MERGE_SORT: mergeSortStep(); break;
        case QUICK_SORT: quickSortStep(); break;
        case HEAP_SORT: heapSortStep(); break;
        }
    }
    glutPostRedisplay();
    glutTimerFunc(DELAY, timer, 0);
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
    case 'r':
        initArray();
        break;
    case 'p':
        paused = !paused;
        break;
    case '1':
        initArray(); algorithm = BUBBLE_SORT; sorting = true;
        strcpy(algorithmName, "Bubble Sort");
        strcpy(complexityText, "Time: O(n^2), Space: O(1)");
        break;
    case '2':
        initArray(); algorithm = SELECTION_SORT; sorting = true;
        strcpy(algorithmName, "Selection Sort");
        strcpy(complexityText, "Time: O(n^2), Space: O(1)");
        break;
    case '3':
        initArray(); algorithm = INSERTION_SORT; sorting = true;
        strcpy(algorithmName, "Insertion Sort");
        strcpy(complexityText, "Time: O(n^2), Space: O(1)");
        break;
    case '4':
        initArray(); algorithm = MERGE_SORT; sorting = true;
        strcpy(algorithmName, "Merge Sort");
        strcpy(complexityText, "Time: O(n log n), Space: O(n)");
        break;
    case '5':
        initArray(); algorithm = QUICK_SORT; sorting = true;
        strcpy(algorithmName, "Quick Sort");
        strcpy(complexityText, "Time: O(n log n), Space: O(log n)");
        break;
    case '6':
        initArray(); algorithm = HEAP_SORT; sorting = true;
        strcpy(algorithmName, "Heap Sort");
        strcpy(complexityText, "Time: O(n log n), Space: O(1)");
        break;
    case 27:
        exit(0);
    }
}

void initOpenGL() {
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Sorting Algorithm Visualizer");

    initOpenGL();
    initArray();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(DELAY, timer, 0);
    glutMainLoop();
    return 0;
}
