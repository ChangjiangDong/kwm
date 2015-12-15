#include "kwm.h"

extern std::vector<window_info> WindowLst;
extern int CurrentSpace;

node_container LeftVerticalContainerSplit(screen_info *Screen, tree_node *Node)
{
    node_container LeftContainer;

    LeftContainer.X = Node->Container.X;
    LeftContainer.Y = Node->Container.Y;
    LeftContainer.Height = Node->Container.Height;

    if(DoesSpaceExistInMapOfScreen(Screen))
        LeftContainer.Width = (Node->Container.Width / 2) - (Screen->Space[CurrentSpace].VerticalGap / 2);
    else
        LeftContainer.Width = (Node->Container.Width / 2) - (Screen->VerticalGap / 2);
    
    return LeftContainer;
}

node_container RightVerticalContainerSplit(screen_info *Screen, tree_node *Node)
{
    node_container RightContainer;

    RightContainer.Y = Node->Container.Y;
    RightContainer.Height = Node->Container.Height;

    if(DoesSpaceExistInMapOfScreen(Screen))
    {
        RightContainer.X = Node->Container.X + (Node->Container.Width / 2) + (Screen->Space[CurrentSpace].VerticalGap / 2);
        RightContainer.Width = (Node->Container.Width / 2) - (Screen->Space[CurrentSpace].VerticalGap / 2);
    }
    else
    {
        RightContainer.X = Node->Container.X + (Node->Container.Width / 2) + (Screen->VerticalGap / 2);
        RightContainer.Width = (Node->Container.Width / 2) - (Screen->VerticalGap / 2);
    }

    return RightContainer;
}

node_container UpperHorizontalContainerSplit(screen_info *Screen, tree_node *Node)
{
    node_container UpperContainer;

    UpperContainer.X = Node->Container.X;
    UpperContainer.Y = Node->Container.Y;
    UpperContainer.Width = Node->Container.Width;

    if(DoesSpaceExistInMapOfScreen(Screen))
        UpperContainer.Height = (Node->Container.Height / 2) - (Screen->Space[CurrentSpace].HorizontalGap / 2);
    else
        UpperContainer.Height = (Node->Container.Height / 2) - (Screen->HorizontalGap / 2);

    return UpperContainer;
}

node_container LowerHorizontalContainerSplit(screen_info *Screen, tree_node *Node)
{
    node_container LowerContainer;

    LowerContainer.X = Node->Container.X;
    LowerContainer.Width = Node->Container.Width;

    if(DoesSpaceExistInMapOfScreen(Screen))
    {
        LowerContainer.Y = Node->Container.Y + (Node->Container.Height / 2) + (Screen->Space[CurrentSpace].HorizontalGap / 2);
        LowerContainer.Height = (Node->Container.Height / 2) - (Screen->Space[CurrentSpace].HorizontalGap / 2);
    }
    else
    {
        LowerContainer.Y = Node->Container.Y + (Node->Container.Height / 2) + (Screen->HorizontalGap / 2);
        LowerContainer.Height = (Node->Container.Height / 2) - (Screen->HorizontalGap / 2);
    }

    return LowerContainer;
}

void CreateNodeContainer(screen_info *Screen, tree_node *Node, int ContainerType)
{
    switch(ContainerType)
    {
        case 1:
        {
            Node->Container = LeftVerticalContainerSplit(Screen, Node->Parent);
        } break;
        case 2:
        {
            Node->Container = RightVerticalContainerSplit(Screen, Node->Parent);
        } break;
        case 3:
        {
            Node->Container = UpperHorizontalContainerSplit(Screen, Node->Parent);
        } break;
        case 4:
        {
            Node->Container = LowerHorizontalContainerSplit(Screen, Node->Parent);
        } break;
    }

    if(ContainerType != -1)
        Node->Container.Type = ContainerType;
}

void CreateNodeContainerPair(screen_info *Screen, tree_node *LeftNode, tree_node *RightNode, int SplitMode)
{
    if(SplitMode == 1)
    {
        CreateNodeContainer(Screen, LeftNode, 1);
        CreateNodeContainer(Screen, RightNode, 2);
    }
    else
    {
        CreateNodeContainer(Screen, LeftNode, 3);
        CreateNodeContainer(Screen, RightNode, 4);
    }
}

tree_node *CreateLeafNode(screen_info *Screen, tree_node *Parent, int WindowID, int ContainerType)
{
    tree_node *Leaf = (tree_node*) malloc(sizeof(tree_node));
    Leaf->Parent = Parent;
    Leaf->WindowID = WindowID;

    CreateNodeContainer(Screen, Leaf, ContainerType);

    Leaf->LeftChild = NULL;
    Leaf->RightChild = NULL;

    return Leaf;
}

tree_node *CreateRootNode()
{
    tree_node *RootNode = (tree_node*) malloc(sizeof(tree_node));
    std::memset(RootNode, '\0', sizeof(tree_node));

    RootNode->WindowID = -1;
    RootNode->Parent = NULL;
    RootNode->LeftChild = NULL;
    RootNode->RightChild = NULL;

    return RootNode;
}

void SetRootNodeContainer(screen_info *Screen, tree_node *Node)
{
    if(DoesSpaceExistInMapOfScreen(Screen))
    {
        Node->Container.X = Screen->X + Screen->Space[CurrentSpace].PaddingLeft;
        Node->Container.Y = Screen->Y + Screen->Space[CurrentSpace].PaddingTop;
        Node->Container.Width = Screen->Width - Screen->Space[CurrentSpace].PaddingLeft - Screen->Space[CurrentSpace].PaddingRight;
        Node->Container.Height = Screen->Height - Screen->Space[CurrentSpace].PaddingTop - Screen->Space[CurrentSpace].PaddingBottom;
    }
    else
    {
        Node->Container.X = Screen->X + Screen->PaddingLeft;
        Node->Container.Y = Screen->Y + Screen->PaddingTop;
        Node->Container.Width = Screen->Width - Screen->PaddingLeft - Screen->PaddingRight;
        Node->Container.Height = Screen->Height - Screen->PaddingTop - Screen->PaddingBottom;
    }

    Node->Container.Type = 0;
}

void CreateLeafNodePair(screen_info *Screen, tree_node *Parent, int LeftWindowID, int RightWindowID, int SplitMode)
{
    Parent->WindowID = -1;

    if(SplitMode == 1)
    {
        Parent->LeftChild = CreateLeafNode(Screen, Parent, LeftWindowID, 1);
        Parent->RightChild = CreateLeafNode(Screen, Parent, RightWindowID, 2);
    }
    else
    {
        Parent->LeftChild = CreateLeafNode(Screen, Parent, LeftWindowID, 3);
        Parent->RightChild = CreateLeafNode(Screen, Parent, RightWindowID, 4);
    }
}

bool IsLeafNode(tree_node *Node)
{
    return Node->LeftChild == NULL && Node->RightChild == NULL ? true : false;
}

tree_node *CreateTreeFromWindowIDList(screen_info *Screen, std::vector<window_info*> *WindowsPtr)
{
    std::vector<window_info*> &Windows = *WindowsPtr;
    tree_node *RootNode = CreateRootNode();
    SetRootNodeContainer(Screen, RootNode);


    if(Windows.size() > 1)
    {
        tree_node *Root = RootNode;
        Root->WindowID = Windows[0]->WID;

        for(int WindowIndex = 1; WindowIndex < Windows.size(); ++WindowIndex)
        {
            while(!IsLeafNode(Root))
            {
                if(!IsLeafNode(Root->LeftChild) && IsLeafNode(Root->RightChild))
                    Root = Root->RightChild;
                else
                    Root = Root->LeftChild;
            }

            DEBUG("CreateTreeFromWindowIDList() Create pair of leafs")
            CreateLeafNodePair(Screen, Root, Root->WindowID, Windows[WindowIndex]->WID, GetOptimalSplitMode(Root));
            Root = RootNode;
        }
    }
    else if(Windows.size() == 1)
    {
        RootNode->WindowID = Windows[0]->WID;
    }
    else
    {
        free(RootNode);
        RootNode = NULL;
    }

    return RootNode;
}

int GetOptimalSplitMode(tree_node *Node)
{
    return (Node->Container.Width / Node->Container.Height) >= 1.618 ? 1 : 2;
}

void SwapNodeWindowIDs(tree_node *A, tree_node *B)
{
    if(A && B)
    {
        DEBUG("SwapNodeWindowIDs() " << A->WindowID << " with " << B->WindowID)
        int TempWindowID = A->WindowID;
        A->WindowID = B->WindowID;
        B->WindowID = TempWindowID;
        ResizeWindowToContainerSize(A);
        ResizeWindowToContainerSize(B);
    }
}

tree_node *GetNodeFromWindowID(tree_node *Node, int WindowID)
{
    if(Node)
    {
        tree_node *CurrentNode = Node;
        while(CurrentNode->LeftChild)
            CurrentNode = CurrentNode->LeftChild;

        while(CurrentNode)
        {
            if(CurrentNode->WindowID == WindowID)
            {
                DEBUG("GetNodeFromWindowID() " << WindowID)
                return CurrentNode;
            }

            CurrentNode = GetNearestNodeToTheRight(CurrentNode);
        }
    }

    return NULL;
}

void ResizeNodeContainer(screen_info *Screen, tree_node *Node)
{
    if(Node)
    {
        if(Node->LeftChild)
        {
            CreateNodeContainer(Screen, Node->LeftChild, Node->LeftChild->Container.Type);
            ResizeNodeContainer(Screen, Node->LeftChild);
        }

        if(Node->RightChild)
        {
            CreateNodeContainer(Screen, Node->RightChild, Node->RightChild->Container.Type);
            ResizeNodeContainer(Screen, Node->RightChild);
        }
    }
}

tree_node *GetNearestNodeToTheLeft(tree_node *Node)
{
    if(Node)
    {
        if(Node->Parent)
        {
            tree_node *Root = Node->Parent;
            if(Root->LeftChild != Node)
            {
                if(IsLeafNode(Root->LeftChild))
                {
                    return Root->LeftChild;
                }
                else
                {
                    Root = Root->LeftChild;
                    while(!IsLeafNode(Root->RightChild))
                        Root = Root->RightChild;
                }

                return Root->RightChild;
            }
            else
            {
                return GetNearestNodeToTheLeft(Root);
            }
        }
    }

    return NULL;
}

tree_node *GetNearestNodeToTheRight(tree_node *Node)
{
    if(Node)
    {
        if(Node->Parent)
        {
            tree_node *Root = Node->Parent;
            if(Root->RightChild != Node)
            {
                if(IsLeafNode(Root->RightChild))
                {
                    return Root->RightChild;
                }
                else
                {
                    Root = Root->RightChild;
                    while(!IsLeafNode(Root->LeftChild))
                        Root = Root->LeftChild;
                }

                return Root->LeftChild;
            }
            else
            {
                return GetNearestNodeToTheRight(Root);
            }
        }
    }

    return NULL;
}

void SwapNodeChildPositions(tree_node *Node)
{
    if(Node && Node->LeftChild && Node->RightChild)
    {
        if(Node->LeftChild->Container.Type == 1)
        {
            tree_node *Temp = Node->LeftChild;
            Node->LeftChild = Node->RightChild;
            Node->RightChild = Temp;
        }

        SwapNodeChildPositions(Node->LeftChild);
        SwapNodeChildPositions(Node->RightChild);
    }
}

void CreateNodeContainers(screen_info *Screen, tree_node *Node)
{
    if(Node && Node->LeftChild && Node->RightChild)
    {
        int SplitMode = GetOptimalSplitMode(Node);
        CreateNodeContainerPair(Screen, Node->LeftChild, Node->RightChild, SplitMode);

        CreateNodeContainers(Screen, Node->LeftChild);
        CreateNodeContainers(Screen, Node->RightChild);
    }
}

void ApplyNodeContainer(tree_node *Node)
{
    if(Node)
    {
        if(Node->WindowID != -1)
            ResizeWindowToContainerSize(Node);

        if(Node->LeftChild)
            ApplyNodeContainer(Node->LeftChild);

        if(Node->RightChild)
            ApplyNodeContainer(Node->RightChild);
    }
}

void DestroyNodeTree(tree_node *Node)
{
    if(Node)
    {
        if(Node->LeftChild)
            DestroyNodeTree(Node->LeftChild);

        if(Node->RightChild)
            DestroyNodeTree(Node->RightChild);

        free(Node);
    }
}
