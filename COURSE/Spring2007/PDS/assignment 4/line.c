#include<stdio.h>

main(){

    int x0,y0,x1,y1;	//Variables used to store Coordinates of line 1
    int p0,q0,p1,q1;	//Variable used to store Coordinates of line 2
    int u0,v0,u1,v1;	//Variable used to store  Coordinates of line 3

    int xi,yi; 		//Stores the point of intersection between line 1 and 2
   
    int m1,m2,m3;// stores slope of line 1 2 and 3
    int m4;		// stores slope of the new line segment 


    int c1,c2; // y=m*x+c

    printf("enter x,y coordinate for line 1\n");
    scanf("%d%d%d%d",&x0,&y0,&x1,&y1);
    
    printf("enter x,y coordinate for line 2\n");
    scanf("%d%d%d%d",&p0,&q0,&p1,&q1);
    
    printf("enter x,y coordinate for line 3\n");
    scanf("%d%d%d%d",&u0,&v0,&u1,&v1);
    
// calculate slope of line 1, 2 & 3
    if((x1-x0)==0) 
	m1=0;
    else
	m1=(y1-y0)/(x1-x0); //slope of line 1
   if((p1-p0)==0)
       m2=0;
   else
       m2=(q1-q0)/(p1-p0); //slope of line 2
   if((u1-u0)==0)
       m3=0;
   else
       m3=(v1-v0)/(u1-u0); //slope of line 3
// Slope calculation ends here

    c1=y0-m1*x0;	
    c2=p1-m2*p0;	

//calculate the point of intersection
    
    xi=-(c1-c2)/(m1-m2);
    yi=c1+m1*xi;
    
//point of intersection calculation ends here
    
    m4=(yi-v0)/(xi-u0); //calculate slope of the line segment (u0,v0) and (xi,yi)
   
//if m4==m3 then the three lines intersect at the same point
    if(m3==m4)
	printf("\nThe Three Line intersect at the same point\n");
    else
	printf("The Three Line does not intersect at the same point\n");
	
  
}

Try with inputs like
	
a)(0,0),(4,4)
  (0,2),(0,-2)
  (-2,2),(2,-2)

b)(-1,-2),(4,5)
  (-2,2),(-2,-2)
  (0,2),(0,-2)

