#include "maxint.h"


int main()
{
    ME_Model model;
    model.load_from_file("model");
    int ncols = 7;
    char *filename = argv[1];
    int buffer = 300;
    char *buf = (char*) malloc( buffer * sizeof(char));
    FILE *fp;
    int count = 0;
    if ( ( fp = fopen( filename, "r" ) ) != NULL )
    {
        while ( fgets( buf, buffer, fp ) != NULL ) {
            ME_Sample s;
            double *spl = split(buf, '\t', ncols);
            for(unsigned char i=0; i<ncols; i++) {
                char *mark = (char*) malloc(10 * sizeof(char));
                sprintf(mark,"feat%d",i);               
                s.add_feature(mark,spl[i]);
            }
            vector<double> vp = model.classify(s);
            for (int i = 0; i < model.num_classes(); i++) {
                cout << vp[i] << "\t" <<  model.get_class_label(i) << endl;
            }
            cout << endl;            
            free(spl);
            break;
        }                
        fclose( fp );
    }
    else
    {
        fprintf( stderr, "Error opening file %s\n", filename );
        free(buf);
        return 1;
    }
}



