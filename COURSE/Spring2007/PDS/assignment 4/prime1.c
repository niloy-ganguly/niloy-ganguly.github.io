#include<stdio.h>
#include <math.h> 

   main()
    {
        int a,c=0,i,n;
        printf("enter the number to be checked");
        scanf("%d",&n);
        for(i=2;i<=sqrt(n);i++)
           {
                 a=n%i;
                 if(a==0)
		     c=c+1;
           }
        if (c>=1)
            printf("the given number is not prime");
        else
            printf("the given number is prime");

   }
