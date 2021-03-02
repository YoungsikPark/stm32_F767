Readme History

it is Personal study for stm32F767 timer function
in this source have 4 program timer problems.

1. first 
2. second 
3. third 
4. four

20200120 git first pull and commit 

2020.01.20 afternoon add contents 
1. program waring point. 
/*we used timer counter it is very very difficult. 
if you dont know useing timer, show youtube or book and study hard!*/ 

timer register used only 65535(0xffff) 
we Usually htim7.Init.Prescaler and htim7.Init.Period calculated and used 
but each value is lmited 65535 
maybe you used 100000 -> overflow value  


There is no other today. 



History Set
2021.03.02
cubemx version change
ETH init -> hal Err

so used Debuger and Trace problem
I found HAL_ETH_Init -> Wait for software reset
Line 263. There are not reset. and set HAL_ETH_STATE_TIMEOUT
We Check this.