
            (function() {
                const cnxWindow = window.parent;
                const gcnx = cnxWindow['cnx'];

                if (!gcnx || !gcnx.cmd) {
                    cnxWindow['cnx'] = {};
                    cnxWindow['cnx'].cmd = [];
                }
                
                window.cnx_data_elements={"ver":146713687,"eu":true,"params":"?cid=016551d5-7095-47c0-a46b-fd0cb9bf4c72","domain":"connatix.com","publisherExclusionLevel":0,"cmpIds":[2,5,6,7,9,10,14,18,21,25,27,28,31,35,46,47,54,58,59,61,63,68,72,76,77,79,90,92,104,105,112,113,123,125,134,162,167,168,171,181,185,198,200,212,213,218,220,222,224,225,227,229,231,235,236,237,242,246,247,258,259,260,264,273,279,280,287,291,292,294,297,299,300,302,303,304,306,308,309,311,312,317,318,321,323,327,329,330,332,335,340,341,343,345,348,350,351,352,354,355,361,363,364,367,371,374,376,379,380,382,383,384,385,386,387,388,390,392,396,397,399,401,403,404,405,407,410,411,412,413,414,415,416,417,418,419,420,421,423,425,426,428,429,430,431,432,433,434,435,436,437,438,440,441,443,445,446,447,448,449,450,451,452,453,454,456,457,459,462,463,471,472,473,474,475,480,481,482,483,491,493],"clientAb":{},"clientAbSettings":{"0":[{"Setup":[0,100]},{"Setup":[1,0]}],"1":[{"Setup":[1,50]},{"Setup":[2,50]}],"2":[{"Setup":[0,99]},{"Setup":[1,1]}],"3":[{"Setup":[0,100]},{"Setup":[1,0]}],"4":[{"Setup":[0,50]},{"Setup":[1,50]}],"5":[{"Setup":[0,100]},{"Setup":[401,0]},{"Setup":[501,0]},{"Setup":[502,0]},{"Setup":[503,0]},{"Setup":[504,0]},{"Setup":[505,0]}]},"playerSettings":{},"serverAbSettings":{"0":[[1350,5],[350,95]],"1":[[1450,50],[450,50]],"2":[[0,50],[0,50]],"3":[[0,50],[0,50]],"4":[[418,50],[1418,50]],"5":[[553,0],[0,100],[443,0]]},"tier":2,"bundleAbSettings":{"abId":"m-blm20-hist2-opt3-dg-rev5","part":"Master"},"hasCookieOptOut":false,"es6":true,"omid":false,"standalone":false};
                window.cnx_data_elements.loadedTimestamp = Date.now();
                let s = window.document.createElement('script');
                s.src = '//cds.connatix.com/p/146713687/elLoader.js';
                s.setAttribute('async', '1');
                s.setAttribute('type', 'text/javascript');
                window.document.body.appendChild(s);
                
            })();