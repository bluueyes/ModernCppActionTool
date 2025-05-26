#include <vector>
#include "concept.h"
#include <atomic>
#include "reaction/utility.h"
#include <functional>

namespace reaction{

    class ObserverNode : public std::enable_shared_from_this<ObserverNode>
    {
    public:
        virtual void valueChange(){

            this->notify();
        }
        
        virtual ~ObserverNode(){}


        void addObserver(ObserverNode* observer){
            m_observers.emplace_back(observer);
        }

        template<typename... Args>
        void updateObserver(Args&&... args){
            (void)(...,args.getPtr()->addObserver(this));
        }

        void notify(){
            for( auto& observer :m_observers ){
                observer->valueChange();
            }
        }
        

    private:
        std::vector<ObserverNode*> m_observers;

    };

    class ObserverGraph{
    public:
        static ObserverGraph& getInstance(){
            static ObserverGraph instance;
            return instance;
        }

        void addNode(std::shared_ptr<ObserverNode> node){
            m_nodes.insert(node);
        }

        void removeNode(std::shared_ptr<ObserverNode> node){
            m_nodes.erase(node);
        }

        void notifyAll(){
            for(auto& node : m_nodes){
                node->notify();
            }
        }
        
    private:
        ObserverGraph() = default;
        std::unordered_set<std::shared_ptr<ObserverNode>> m_nodes;
    };

    class FieldGraph{
    public:
        static FieldGraph& getInstance(){
            static FieldGraph instance;
            return instance;
        }

        void addObj(uint64_t id, std::shared_ptr<ObserverNode> node){
            m_fieldMap[id].insert(node);
        }

        void removeObj(uint64_t id){
            m_fieldMap.erase(id);
        }

        void bindField(uint64_t id, std::shared_ptr<ObserverNode> objPtr){
            if(! m_fieldMap.count(id) ){
                return;
            }

            for(auto &node : m_fieldMap[id]){

                node->addObserver(objPtr.get());
            }
        }

    private:
        FieldGraph() = default;
        std::unordered_map<uint64_t,std::unordered_set<std::shared_ptr<ObserverNode>>> m_fieldMap;
    
    };
}