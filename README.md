# DataCenter_CS776

To make the simulation run faster the specified times are broken down to a ratio of 3,600:1
- 1 hour (3,600,000 ms; 3,600,000,000 microseconds) = 1 second (1,000 ms; 1,000,000 microseconds)
- 1 second (1,000 ms; 1,000,000 microseconds) = 2.7778 ms (~2778 microseconds)


Strategy 1 - Ample lead up time to ensure everything is migrated:
- At the 30 minute (simulation: 0.5 seconds) mark everyone in that shift begins to dump there progress to another available car or to another car's hard drive if none are available.
- As new cars come in those left over jobs are picked up and completed 

Strategy 2 - 
- Every 10 minutes (simulation: 0.1 seconds) every car in the shift that is about to leave is checked to see how much time the migration will take
- If one is already within 1 minute (simulation: 0.01 seconds) of not being able to get everything transferred the transfer will begin 
- The rest will schedule themselves based on who needs the most time (i.e. how much data they have)