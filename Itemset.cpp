#include "Itemset.h"
#include "ItemsetTable.h"
#include "Dataset.h"

/* Itemset functions implementation */
Itemset::Itemset(std::string RHSItemset){
    
    itemset = RHSItemset;
    utility = 0;
    support = 0;
    TWU     = 0;
    prev    = NULL;
    next    = NULL;
}

void Itemset::genIDList(Dataset& dataset, ItemsetTable& itemsetTable){
    
    int itemsetLen = itemset.length();

    if(itemsetLen == 1){
        
        /* if itemset Length == 1
           need to construct ID List from original dataset
        */
        for(int i=1 ; i<=dataset.retNumOfTransac() ; i++){
            
            if( dataset.existInTransac(itemset, i) )
                idList.insert(i);
        }
    }
    else{
        
        /* if itemset length > 1
           construct ID List from its subset's ID List

           IDList(ABC)
           intersact IDList(AB) with IDList(AC)
        */
        std::string subItemset1 = itemset.substr(0, itemsetLen-2);
        std::string subItemset2 = subItemset1;

        subItemset1 += itemset[itemsetLen-2];
        subItemset2 += itemset[itemsetLen-1];
        
        Itemset* itemsetCursor1 = itemsetTable.headList[ itemsetLen-1 ];
        Itemset* itemsetCursor2 = itemsetTable.headList[ itemsetLen-1 ];
        
        /* get subset 1 */
        while( itemsetCursor1 ){
            
            if(itemsetCursor1->itemset == subItemset1)
                break;

            itemsetCursor1 = itemsetCursor1->next;
        }
                
        /* get subset 2 */
        while( itemsetCursor2 ){
            
            if(itemsetCursor2->itemset == subItemset2)
                break;

            itemsetCursor2 = itemsetCursor2->next;
        }
        
        /* intersact the 2 IDLists */
        for(std::set<int>::iterator It = itemsetCursor1->idList.begin() ;
            It != itemsetCursor1->idList.end() ;
            It++){
                    
            if( itemsetCursor2->idList.count(*It) == 1 )
                idList.insert(*It);
        }
    }
    
    /* set itemset's support */
    support = idList.size();
}

void Itemset::genUtilList(Dataset& dataset, ItemsetTable& itemsetTable){
    
    int itemsetLen = itemset.length();

    if(itemsetLen == 1){
        
        /* if itemset length == 1
           construct UtilList from its IDList
        */
        for(std::set<int>::iterator It = idList.begin() ;
            It != idList.end() ;
            It++){
            
            int itemUtil = dataset.retItemUtilInTransac(itemset, *It);
            utilList.insert( std::pair<int, int>(*It, itemUtil) );
            utility += itemUtil;
        }
    }
    else{
        
        /* if itemset length > 1
           construct UtilList from its subsets
        */

        /* prev layer: 2 cursors */
        std::string subItemset1 = itemset.substr(0, itemsetLen-2);
        std::string subItemset2 = subItemset1;

        subItemset1 += itemset[itemsetLen-2];
        subItemset2 += itemset[itemsetLen-1];
        
        Itemset* itemsetCursor1 = itemsetTable.headList[ itemsetLen-1 ];
        Itemset* itemsetCursor2 = itemsetTable.headList[ itemsetLen-1 ];

        /* get subset 1 */
        while( itemsetCursor1 ){
            
            if(itemsetCursor1->itemset == subItemset1)
                break;

            itemsetCursor1 = itemsetCursor1->next;
        }
                
        /* get subset 2 */
        while( itemsetCursor2 ){
            
            if(itemsetCursor2->itemset == subItemset2)
                break;

            itemsetCursor2 = itemsetCursor2->next;
        }

        if(itemsetLen == 2){

            /* if itemset len == 2
               only have to handle the previous layer
            */
            for(std::set<int>::iterator It = idList.begin() ;
                It != idList.end() ;
                It++){
                        
                int itemsetUtil = itemsetCursor1->utilList[*It] + itemsetCursor2->utilList[*It];

                utilList.insert( std::pair<int,int>(*It, itemsetUtil) );
                utility += itemsetUtil;
            }
        }
        else{

            /* if itemset len == 2
               the second previous layer is also needed
            */

            /* second prev layer: 1 cursor */
            std::string subItemset3 = itemset.substr(0, itemsetLen-2);

            Itemset* itemsetCursor3 = itemsetTable.headList[ itemsetLen-2 ];

            /* get subset that has the same prefix of the 2 subsets  */
            while( itemsetCursor3 ){
                
                if(itemsetCursor3->itemset == subItemset3)
                    break;

                itemsetCursor3 = itemsetCursor3->next;
            }
            
            /* construct UtilList from the three subset's util lists
               
               util(ABC, T3) = util(AB, T3)
                             + util(AC, T3)
                             - util(A, T3)
            */
            for(std::set<int>::iterator It = idList.begin() ;
                It != idList.end() ;
                It++){
                        
                int itemsetUtil = itemsetCursor1->utilList[*It]
                                + itemsetCursor2->utilList[*It]
                                - itemsetCursor3->utilList[*It];

                utilList.insert( std::pair<int,int>(*It, itemsetUtil) );
                utility += itemsetUtil;
            }
        }
    }
}

bool Itemset::isSubset(std::string subset){
    
    /* check if string(subset) is string(itemset) */
    for(int i=0 ; i<subset.length() ; i++){
       
        std::size_t foundPos = itemset.find(subset[i]);
        
        /* char subset[i] not found in itemset */
        if( foundPos == std::string::npos )
            return false;
    }

    return true;
}

bool Itemset::isHUG(int minUtil, ItemsetTable& itemsetTable){
    
    /* prune Low util itemset */
    if( utility < minUtil )
        return false;

    /* check if it is genertor:
       all support(subset) must be greater than its support
    */
    int itemsetLen = itemset.length();

    Itemset* itemsetCursor = itemsetTable.headList[ itemsetLen-1 ];

    while(itemsetCursor){
        
        if( isSubset(itemsetCursor->itemset)
         && itemsetCursor->support == support )
            return false;

        itemsetCursor = itemsetCursor->next;
    }

    return true;
}

void Itemset::computeTWU(Dataset& dataset){
    
    /* simply sum up all the TU of its IDList */
    for(std::set<int>::iterator It = idList.begin() ;
        It != idList.end() ;
        It++){

        TWU += dataset.retTransacUtil(*It);
    }
}

int Itemset::retTWU(){
    
    return TWU;
}

void Itemset::printIDList(){

    for(std::set<int>::iterator It = idList.begin() ;
        It != idList.end() ;
        It++){
    
        std::cout << *It << " ";
    }

    std::cout << std::endl;
}

void Itemset::printItemsetDetails(){
    
    std::cout << itemset << " " << utility << " " << support << " " << TWU <<std::endl;
}