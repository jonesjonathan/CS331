////////////////////////////////////////////////////////////////////////////
//  Jonathan Jones (jonesjon) and Brett Case (casebr)
//  Programming Assignment 1
//  CS331 Intro to Artificial Intelligence Spring2018
////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>

using namespace std;

struct Bank {
    unsigned int num_wolves, num_chickens, boat;
};

struct State {
    Bank left, right;

    bool operator==(const State a) const
    {
        return ((left.num_chickens == a.left.num_chickens)
        && (left.num_wolves == a.left.num_wolves)
        && (left.boat == a.left.boat)
        && (right.num_chickens == a.right.num_chickens)
        && (right.num_wolves == a.right.num_wolves)
        && (right.boat == a.right.boat));
    }
};

struct Node {
    State state;
    Node* prev;
};


Node* bfs(State, State, int&);
void dfs();
void iddfs();
void astar();

void file_to_state(State&, State&, ifstream&, ifstream&);
void show_state(State);
void write_state_to_file(State);
bool isValid(State);
bool enoughAnimals(State, int);
bool moveAnimals(State, State*,  int, int);
bool visited(vector<State>, State);
void insert_bfs_node(Node*& parent, Node*& child, State s, vector<State>& states, queue<Node*>& n_queue);
void print_solution(Node*, int);


//Globals
ofstream out_f;

// <initial state file> <goal state file> <mode> <output file>
int main(int argc, char** argv){
    //Initialize command line argument buffers
    string init_state_file, goal_state_file, mode, output_file;
    State initial;
    State goal;
    Node* tree = NULL;

    //Check for valid number of arguments
    if(argc == 5)
        init_state_file = argv[1], goal_state_file = argv[2], mode = argv[3], output_file = argv[4];
    else 
    {
        cout << "USAGE: Invalid argument count\n<initial state file> <goal state file> <mode> <output file>" << endl;
        return 1;
    }

    //Open files for reading and writing
    ifstream init_f(init_state_file.c_str()), goal_f(goal_state_file.c_str());
    out_f.open(output_file.c_str());

    if(!(init_f.is_open() && goal_f.is_open() && out_f.is_open())) //Check if files were opened correctly
    {
        cout << "ERROR: Invalid filenames provided" << endl;
        return 1;
    }

    //Fill initial and goal states
    file_to_state(initial, goal, init_f, goal_f);

    //Close input streams
    init_f.close(); goal_f.close();

    //Print out the solution to stdout and the output file
    int expanded;
    Node* solution = bfs(initial, goal, expanded);
    print_solution(solution, expanded);

    return 0;
}

/***************************************************************
* Function: print_solution
* Description: Prints the solution path of the problem, the number of nodes expanded and the number of nodes in the solution to stdout and the output file. 
* Backtraces the node to the root to print the soluton out BACKWARDS.
* Params: Solution node that is linked back to the root node through associations.
* Returns: None
* Pre-Conditions: Output stream is open
* Post-Conditions: Info is printed to stdout and added to the output file
**************************************************************/
void print_solution(Node* solution, int expanded)
{
    int depth = 0;
    if(solution == NULL)
    {
        cout << "Nodes expanded" << expanded << "\nNo solution found." << endl;
        out_f << "Nodes expanded" << expanded << "\nNo solution found." << endl;
    }
    else
    {
        cout << "END OF PATH (Goal state below)" << endl;
        out_f << "END OF PATH (Goal state below)" << endl;
        while(solution != NULL)
        {
            depth++;
            show_state(solution->state);
            write_state_to_file(solution->state);
            solution = solution->prev;
        }
        cout << "PATH BEGINS (Initial state above)" << endl;
        out_f << "PATH BEGINS (Initial state above)" << endl;

        cout << "Nodes expanded: " << expanded << "\nNodes in solution: " << depth << endl;
        out_f << "Nodes expanded: " << expanded << "\nNodes in solution: " << depth << endl;
    }
}

/***************************************************************
* Function: file_to_state
* Description: Reads from provided input streams into the initial and goal states
* Params: initial state, goal state, initial file stream, goal file stream
* Returns: none
* Pre-Conditions: Streams are open
* Post-Conditions: States are filled
**************************************************************/
void file_to_state(State& i_state, State& g_state, ifstream& init, ifstream& goal)
{
    char a, b; //Dummy variables
    init >> i_state.left.num_chickens>> a >> i_state.left.num_wolves >> b >> i_state.left.boat;
    init >> i_state.right.num_chickens >> a >> i_state.right.num_wolves >> b >> i_state.right.boat;

    goal >> g_state.left.num_chickens >> a >> g_state.left.num_wolves >> b >> g_state.left.boat;
    goal >> g_state.right.num_chickens >> a >> g_state.right.num_wolves >> b >> g_state.right.boat;
}

/***************************************************************
* Function: show_state
* Description: Prints the state to stdout
* Params: State to be printed
* Returns: None
* Pre-Conditions: State contains data
* Post-Conditions: None
**************************************************************/
void show_state(State s)
{
    cout << s.left.num_chickens << s.left.num_wolves << s.left.boat << 
    "," << s.right.num_chickens << s.right.num_wolves << s.right.boat << endl;
}

/***************************************************************
* Function: write_state_to_file
* Description: Writes the contents of the state in a formatted manner into the output stream
* Params: State to be written to file
* Returns: None
* Pre-Conditions: State contains data
* Post-Conditions: None
**************************************************************/
void write_state_to_file(State s)
{
    out_f << s.left.num_chickens << "," << s.left.num_wolves << "," << s.left.boat << '\n'
    << s.right.num_chickens << "," << s.right.num_wolves << "," << s.right.boat << '\n' << endl;
}

/***************************************************************
* Function: isValid
* Description: Determines the validity of the current state. If there are more wolves than chickens on either bank, the state is invalid.
* Params: State to be validated
* Returns: True if valid state, false if invalid state
* Pre-Conditions: State contains data
* Post-Conditions: None
**************************************************************/
bool isValid(State s)
{
    return ((s.left.num_chickens >= s.left.num_wolves || s.left.num_chickens == 0) && (s.right.num_chickens >= s.right.num_wolves || s.right.num_chickens == 0));
}

//pass in the number of animals on the bank and the amount to try to put on the boat
//to see if this is a valid move
/***************************************************************
* Function: 
* Description: 
* Params: 
* Returns: 
* Pre-Conditions: 
* Post-Conditions: 
**************************************************************/
bool enoughAnimals(int numAnimals, int amountToMove)
{
    return numAnimals >= amountToMove;
}
//find out what side boat is on and move specificed number of animals across
/***************************************************************
* Function: 
* Description: 
* Params: 
* Returns: 
* Pre-Conditions: 
* Post-Conditions: 
**************************************************************/
bool moveAnimals(State s, State* newNode, int chicks, int wolves){
    if(s.left.boat == 1){ //left side has boat
    // cout << "left side" << endl;
    //move number of chickens
    if(enoughAnimals(s.left.num_chickens, chicks)){
        newNode->left.num_chickens = s.left.num_chickens - chicks;
        newNode->right.num_chickens = s.right.num_chickens + chicks;
    }else return false; //if cant make move return false

    //move number of wolves
    if(enoughAnimals(s.left.num_wolves, wolves)){
        newNode->left.num_wolves = s.left.num_wolves - wolves;
        newNode->right.num_wolves = s.right.num_wolves + wolves;
    }else return false; //if cant make move return false

    //move boat
    newNode->left.boat = 0;
    newNode->right.boat = 1;
    return true;

    }else{ //right side has boat
    // cout << "right side" << endl;
    //move number of chickens
    if(enoughAnimals(s.right.num_chickens, chicks)){
        newNode->right.num_chickens = s.right.num_chickens - chicks;
        newNode->left.num_chickens = s.left.num_chickens + chicks;
    }else return false; //if cant make move return false

    //move number of wolves
    if(enoughAnimals(s.right.num_wolves, wolves)){
        newNode->right.num_wolves = s.right.num_wolves - wolves;
        newNode->left.num_wolves = s.left.num_wolves + wolves;
    }else return false; //if cant make the move return false

    //move boat
    newNode->right.boat = 0;
    newNode->left.boat = 1;
    return true;
    }
}

/***************************************************************
* Function: bfs
* Description: Uses a breadth first search algorithm to find the solution path
* Params: Initial state, goal state, int container for number of expanded nodes
* Returns: Solution node that can be backtraced to the root node or NULL if there was no solution
* Pre-Conditions: Valid initial and goal state. Existing int container for num_expanded
* Post-Conditions: Exits
**************************************************************/
Node* bfs(State start, State goal, int& num_expanded)
{
    num_expanded = 1; //We always expand the first node

    queue<Node*> node_queue;  //Queue to hold next expanded nodes to check
    vector<State> unique_states; //Vector to hold already visited states (NOTE: Could change this to nodes so we can free them later and avoid memory leaks)

    //Create a starting node that has the initial state
    Node* node = new Node();
    node->state = start;
    node->prev = NULL;

    //Add this node to both the queue and the visited states
    node_queue.push(node);
    unique_states.push_back(node->state);

    Node* newNode; //Container for any new node we make

    //While there the queue is not empty (Still nodes to expand/view)
    while(!node_queue.empty())
    {
        node = node_queue.front(); //Get the node at the front of the queue for evaluation
        node_queue.pop(); //Physically remove that node from the queue

        //Check if the node's state is the goal state
        if(node->state == goal)
            return node;

        State newState; //Container for any new state we create via moveAnimals()

        //Creates 5 new states and checks if those states are able to be created, are valid states and are not duplicate states
        //Inserts that node into the tree if it passes these cases, updates the visited nodes, the queue and increments num_expanded
        if(moveAnimals(node->state, &newState, 1, 0) && isValid(newState) && !visited(unique_states, newState)) //Move one chicken
        {
            insert_bfs_node(node, newNode, newState, unique_states, node_queue); num_expanded++;
        }
        if(moveAnimals(node->state, &newState, 2, 0) && isValid(newState) && !visited(unique_states, newState)) //Move two chickens
        {
            insert_bfs_node(node, newNode, newState, unique_states, node_queue); num_expanded++;
        }
        if(moveAnimals(node->state, &newState, 0, 1) && isValid(newState) && !visited(unique_states, newState)) //Move one wolf
        {
            insert_bfs_node(node, newNode, newState, unique_states, node_queue); num_expanded++;
        }
        if(moveAnimals(node->state, &newState, 1, 1) && isValid(newState) && !visited(unique_states, newState)) //Move one chicken and one wolf
        {
            insert_bfs_node(node, newNode, newState, unique_states, node_queue); num_expanded++;
        }
        if(moveAnimals(node->state, &newState, 0, 2) && isValid(newState) && !visited(unique_states, newState)) //Move two wolves
        {
            insert_bfs_node(node, newNode, newState, unique_states, node_queue); num_expanded++;
        }
    }
    return NULL; //Return null if no solution was found
}

/***************************************************************
* Function: insert_bfs_node
* Description: Creates a new node with the provided state, parent node associato and adds it to the visited list and the queue. 
* Params: Parent node, new node, new state, closed list of visited states, the queue
* Returns: None
* Pre-Conditions: The state to be added as a new node must be a valid state that hasn't already been added to the tree
* Post-Conditions: Node has been added to the tree
**************************************************************/
void insert_bfs_node(Node*& parent, Node*& child, State s, vector<State>& states, queue<Node*>& n_queue)
{
    child = new Node();
    child->state = s;
    child->prev = parent;
    states.push_back(s);
    n_queue.push(child);
}

/***************************************************************
* Function: visited
* Description: Searches through the closed list of states we've already created and determines if the candidate state is a duplicate or not
* Params: Closed list of states (Vector<State>), Candidate state
* Returns: True if the state is a duplicate, false if otherwise
* Pre-Conditions: Candidate state and states in the list (Could be an empty list) actually contain valid info
* Post-Conditions: None
**************************************************************/
bool visited(vector<State> states, State state)
{
    vector<State>::iterator it;
    for(it = states.begin(); it != states.end(); it++)
    {
        if(*it == state)
            return true;
    }
    return false;
}
