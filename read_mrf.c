#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>
#include <dirent.h>
#include <string.h>

#define MAXLEN 1024 * 1024

typedef struct _dat{
    short year;
    char month;
    char day;
    char hour;
    char minute;
    char second;
} str_time;

typedef struct _objtype {
    int type_id;
    int unknown;
    int length;
} str_objtype;


typedef struct _namespace_header {
    char NE_lable[64];
    char name[400];
    char version[64];
    int measureNumber;
    str_time stime;
    char pad[4];//for align
    int period;
    int classNumber;
} str_namespace_header;

typedef struct _moc_header {
    int moc_id;
    int unknown_1;
    int mu_number;
} str_moc_header;

typedef struct _moc_footer {
    int moi_number;
} str_moc_footer;


typedef struct _h1 {
    int format_version;
    char name[400];
    char sender_type[8];
    char vendor_name[32];
    str_time stime;
    char pad[64];
    int measureDataCount;

} header1;


int little2big(int le) {

    return (le & 0xff) << 24
            | (le & 0xff00) << 8
            | (le & 0xff0000) >> 8
            | (le >> 24) & 0xff;
}

uint64_t little2big_l(uint64_t le) {
    return    (le & 0x00000000000000ff) << 56
            | (le & 0x000000000000ff00) << 40
            | (le & 0x0000000000ff0000) << 24
            | (le & 0x00000000ff000000) << 8
            | (le & 0xff00000000000000) >> 56
            | (le & 0x00ff000000000000) >> 40
            | (le & 0x0000ff0000000000) >> 24
            | (le & 0x000000ff00000000) >> 8;

}

short little2big_s(short le) {

    return (le & 0xff) << 8 | (le & 0xff00) >> 8;
}

int main(int argc, char *argv[]) {
    uint64_t *data = malloc(1024 * 200 * 2400 * sizeof(uint64_t));
    uint64_t *p;
    char *DIR_HOME="/home/zwz/data_compress/compare/mrf/";

    DIR *dirp;
    char datafile[1024];
    strcpy(datafile,DIR_HOME);
    struct dirent **namelist;
    int dirnumber;
    dirnumber = scandir("/home/zwz/data_compress/compare/mrf/", &namelist, 0, alphasort);
    //struct dirent *direntp;
    //dirp = opendir(datafile);
    int currentFile = 0;
    str_time colTime[1000];
    int total_column = 0;
    if (dirnumber < 0)
    {
      perror("not found\n");
    }
    else
    {
      while(dirnumber--)
      {
        /*printf("%s\n", namelist[dirnumber]->d_name);*/
        p = data + currentFile * 1024 * 200;
        //Read directory for all MRFS
        if(namelist[dirnumber]->d_name[0] != '.') {
            strcpy(datafile,DIR_HOME);
            strcat(datafile,namelist[dirnumber]->d_name);
            //printf("Ready to process File[%d]:%s\r\n",currentFile,datafile);
            fflush(stdout);

            FILE *infile;
            infile = fopen(datafile, "rb");
            unsigned char buf[MAXLEN];

            if(infile == NULL) {
                //printf("Open File error:%s\r\n", datafile);
                exit(1);
            }

            int rc;
            header1 hdr1;
            str_namespace_header ns_hdr;
            str_moc_header moc_hdr;
            str_moc_footer ns_ft;
            str_objtype *objtp;
            int filePos = 0;
            int measureDataCount = 0;
            int measDataLength[50];
            int parameterCount = 0;
            uint32_t column = 0;

            rc = fread(&hdr1,sizeof(header1), 1,infile);
            if(rc == 1) {
                //printf("read ok\r\n");
                hdr1.format_version = little2big(hdr1.format_version);
                hdr1.measureDataCount = little2big(hdr1.measureDataCount);
                measureDataCount = hdr1.measureDataCount;
                //printf("MeasureDataCount=%d\r\n",measureDataCount);
                hdr1.stime.year = little2big_s(hdr1.stime.year);
                memcpy(&colTime[currentFile],&hdr1.stime,sizeof(str_time));

                //printf("%d,%s,%d,%d\r\n",hdr1.format_version, hdr1.name, hdr1.measureDataCount,hdr1.stime.year);
            }

            for(int i=0;i<measureDataCount;i++) {
                fread(&measDataLength[i],sizeof(int), 1,infile);
                measDataLength[i] = little2big(measDataLength[i]);
                //printf("measDataLength[%d]=%d\n",i,measDataLength[i]);
            }
            for(int nmspc=0;nmspc<measureDataCount;nmspc++) {
                filePos = 0;
                rc = fread(&ns_hdr,sizeof(ns_hdr), 1,infile);
                filePos += sizeof(ns_hdr);
                if(rc > 0) {
                    //printf("read namespace header\r\n");
                }

                while(1) {
                    rc = fread(&moc_hdr,sizeof(moc_hdr), 1,infile);
                    filePos += sizeof(moc_hdr);
                    if(rc > 0) {
                        //printf("read moc header\r\n");
                        moc_hdr.mu_number = little2big(moc_hdr.mu_number);
                        parameterCount = moc_hdr.mu_number;
                        //printf("mu number(parameter count)=%d\r\n",moc_hdr.mu_number);
                    }

                    objtp = malloc(moc_hdr.mu_number * sizeof(str_objtype));
                    rc = fread(objtp,sizeof(str_objtype), moc_hdr.mu_number,infile);
                    filePos += sizeof(str_objtype) * moc_hdr.mu_number;
                    //printf("%d\r\n",rc);
                    if(rc > 0) {
                        //printf("read header\r\n");
                        for(int i = 0;i < moc_hdr.mu_number;i++) {
                            objtp[i].type_id = little2big(objtp[i].type_id);
                            objtp[i].length = little2big(objtp[i].length);
                            //printf("%d,%d\r\n",objtp[i].type_id, objtp[i].length);
                        }
                    }

                    rc = fread(&ns_ft,sizeof(ns_ft), 1,infile);
                    filePos += sizeof(ns_ft);
                    if(rc > 0) {
                        //printf("read ns_footer\r\n");
                        ns_ft.moi_number = little2big(ns_ft.moi_number);
                        //printf("moi numbers=%d\r\n",ns_ft.moi_number);
                    }

                    for(int objs = 0;objs < ns_ft.moi_number;objs++) {
                        //printf("MOI NUMBER = %d\r\n",objs);
                        fread(&buf,16 * 7, 1,infile);
                        filePos += 16 * 7;
                        //printf("read MOI id String=\"%s\"\n", buf);
                        fflush(stdout);
                        for(int values = 0;values < parameterCount;values++) {
                            if(objtp[values].length == 8) {
                                uint64_t temp = 0;
                                fread(&temp,sizeof(uint64_t), 1,infile);
                                filePos += sizeof(uint64_t);
                                temp = little2big_l(temp);
                                p[column] = temp;
                                //printf("%" PRIu64 "\n", temp);
                            } else if(objtp[values].length == 4) {
                                //printf("read 32bit\r\n");
                                //printf("column = %d\r\n",column);
                                fflush(stdout);
                                int temp = 0;
                                fread(&temp,sizeof(int), 1,infile);
                //printf("filepos = %d,value=%x\r\n", filePos,(int)(p[column]));
                fflush(stdout);
                                filePos += sizeof(int);
                                temp = little2big(temp);
                                p[column] = temp;
                                //printf("%d\n",temp);
                fflush(stdout);
                            }
                            column++;
                total_column = column;
                            //printf("column = %d\r\n",column);
                        }
                        int suspect_f = 0;
                        fread(&suspect_f,sizeof(int), 1,infile);
                        filePos += sizeof(int);
                        //printf("suspect flag=%d\r\n", suspect_f);
                    }
                    //printf("filePos=%d\r\n", filePos);
                    fflush(stdout);
                    if(filePos >= measDataLength[0]) {
                        //printf("finished read namespace\r\n");
                        break;
                    }
                }//while
            }//for
        fclose(infile);
        }
        currentFile++;
        free(namelist[dirnumber]);
    }//while
    free(namelist);
  }//else


    // FILE * outfile, *infile;
    // outfile = fopen("Rout.bin", "wb" );
    // if( outfile == NULL ){
    //     //printf("%s, %s","Rout.bin","not exit/n");
    // }
    // for(int i =0;i < currentFile;i++) {
    //     printf("%04d-%02d-%02d %02d:%2d:00,",colTime[i].year,colTime[i].month,colTime[i].day,colTime[i].hour,colTime[i].minute);
    //     p = data + i * 1024 * 200;
    //     for(int k = 0;k < total_column;k++) {
    //         fwrite(p, sizeof(uint64_t), 1, outfile );
    //         printf("%" PRIu64 " ", p[k]);
    //         printf(",");
    //     }
    //     printf("\r\n");
    // }
    // fclose(outfile);


    FILE * outfile, *infile;
    outfile = fopen("out.bin", "ab" );
    if( outfile == NULL ){
        printf("%s, %s","out.bin","not exit/n");
        exit(1);
    }
    for(int k = 0;k < total_column;k++) {
        for(int i =0;i < currentFile;i++) {
            p = data + i * 1024 * 200;
            fwrite(p[k], sizeof(uint64_t), 1, outfile);
            printf("%" PRIu64 " ", p[k]);
            printf(",");
        }
        printf("\r\n");
    }
    fclose(outfile);


    free(data);
    closedir(dirp);
    return 0;

}
