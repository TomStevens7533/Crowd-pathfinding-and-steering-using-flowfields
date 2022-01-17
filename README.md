# Crowd-pathfinding-and-steering-using-flowfields

This is an implementation of the flowfield pathfinding technique, this technique solves the computation problem of moving thousands of agensts.
A flowfield guides the agents to their destination by telling the agent where to go, this way wwe dont need to calculate a path for every agent

<h1>My implementation  </h1>
  
Every node in this grid has a cost to it, water for example has a cost of 200001 which mean you cant cross it.
 Mud which has a terrain cost of 3 is still higher than dirt which has a cost of 1 so the path will try to avoid Mud unless it is a cheaper option to traverse through the mud instead of going around.
  
<img src="https://user-images.githubusercontent.com/34093176/149738373-5c439c4b-cd61-4efb-b61f-4db312a19479.png">
  
  
<h2>Examples on usecases of the flowfield technique</h2>
 
 flowfield can be used in al different kind of scenarios!
  -strategy games: can be used to control different armies to play smart.
  -city building games: can be used to control the crowd going from one place to to next.
  
  flowfield can almost always be usefull when controlling allot of agents! 
  
 <img src="https://user-images.githubusercontent.com/34093176/149742401-6d79a359-1cc3-48ed-bc84-3e42945cd9c1.gif">
