# Crowd-pathfinding-and-steering-using-flowfields

This is an implementation of the flowfield pathfinding technique. This technique solves the computation problem of moving thousands of agents. This flowfield guides the agents to their destination by telling the agent where to go, this way we dont need to separately calculate a path for every agent. 

<h1>My implementation  </h1>
  
Every node in this grid has a terrain cost. For example: water has a cost of 200001 which mean you cant go across it. Mud, which has a cost of 3, is still higher than solid dirt which has a cost of only 1. Logically the path will try to avoid Mud unless it is a cheaper option to traverse the mud instead of finding an alternate route going around.
  
<img src="https://user-images.githubusercontent.com/34093176/149738373-5c439c4b-cd61-4efb-b61f-4db312a19479.png">
  
  
<h2>Examples on usecases of the flowfield technique</h2>
 
Flowfields can be used in various different  scenarios!
 
 -strategy games: can be used to control different armies to play smarter.                                                                                                 
 -city building games: can be used to direct the crowd going from one place to to the next.
  
  flowfields can almost always be usefull when controlling allot of agents! 
  
 <img src="https://user-images.githubusercontent.com/34093176/149742401-6d79a359-1cc3-48ed-bc84-3e42945cd9c1.gif">
