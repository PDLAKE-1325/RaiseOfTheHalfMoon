#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// 노드와 링크 구조체 정의
typedef struct node node;

typedef struct _link {
    node* target;           // 연결된 노드를 가리킴
    struct _link* next;     // 다음 연결 노드
} _link;

struct node {
    _link* linked_nodes;    // 연결된 노드를 가진 링크 리스트
    int id;                 // 노드 ID (방문 체크용)
};

// 노드 생성 함수
node* createNode(int id) {
    node* newNode = (node*)malloc(sizeof(node));
    newNode->linked_nodes = NULL; // 초기에는 연결된 노드가 없음
    newNode->id = id;
    return newNode;
}

// 링크 생성 함수
_link* createLink(node* target) {
    _link* newLink = (_link*)malloc(sizeof(_link));
    newLink->target = target;
    newLink->next = NULL;
    return newLink;
}

// DFS 함수 (재귀적 깊이 우선 탐색)
void dfs(node* currentNode, int* visited, int numNodes) {
    if (currentNode == NULL) {
        return;
    }

    // 방문 여부 체크
    if (currentNode->id < 0 || currentNode->id >= numNodes || visited[currentNode->id]) {
        return;
    }

    // 현재 노드를 방문했다고 표시
    visited[currentNode->id] = 1;

    // 현재 노드 방문 출력
    printf("노드 방문: %d\n", currentNode->id);

    // 연결된 노드를 따라 DFS 수행
    _link* link = currentNode->linked_nodes;
    while (link != NULL) {
        dfs(link->target, visited, numNodes); // 연결된 노드로 DFS
        link = link->next;
    }
}

// 두 노드를 연결하는 함수
void addLink(node* fromNode, node* toNode) {
    _link* newLink = createLink(toNode);
    newLink->next = fromNode->linked_nodes;
    fromNode->linked_nodes = newLink;
}

int main() {
    // 노드 생성
    node* node1 = createNode(0);
    node* node2 = createNode(1);
    node* node3 = createNode(2);
    node* node4 = createNode(3);

    // 노드 연결
    addLink(node1, node2);
    addLink(node1, node3);
    addLink(node2, node4);

    // 방문 기록 배열 (노드마다 방문 여부 체크)
    int numNodes = 4; // 총 노드 개수
    int visited[numNodes]; // 각 노드 방문 여부를 기록하는 배열

    // 방문 여부 배열 초기화
    for (int i = 0; i < numNodes; i++) {
        visited[i] = 0;
    }

    // DFS 시작
    printf("DFS 탐색 시작:\n");
    dfs(node1, visited, numNodes);

    // 메모리 해제
    free(node1);
    free(node2);
    free(node3);
    free(node4);

    return 0;
}
