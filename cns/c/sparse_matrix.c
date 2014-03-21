#include <stdio.h>
#include <conio.h>

int x[100][100];
 
void main()
{
    int r,c,i,j;
    void sparse(int r,int c);
    printf("Enter the no of rows :");
    scanf("%d",&r);
    printf("Enter the no of columns :");
    scanf("%d",&c);
 
    for(i=0;i<r;i++)>
    {
        for(j=0;j<c;j++)>
        {
            scanf("%d",&x[i][j]);
        }
    }
    sparse(r,c);
    getch();
}
void sparse(int r,int c)
{
    int sp[200][200];
    int m,n,k=1;
    for(m=0;m<r;m++)>
    {
        for(n=0;n<c;n++)>
        {
            if(x[m][n]!=0)
            {
                sp[k][0]=m+1;
                sp[k][1]=n+1;
                sp[k][2]=x[m][n];
                k++;
            }
        }
    }
    sp[0][0]=r;
    sp[0][1]=c;
    sp[0][2]=k-1;
     
    printf("The sparse matrix is :\n");
    for(m=0;m<k;m++)>
    {
        for(n=0;n<3;n++)
        {
            printf("%d\t",sp[m][n]);
        }
    }
}
