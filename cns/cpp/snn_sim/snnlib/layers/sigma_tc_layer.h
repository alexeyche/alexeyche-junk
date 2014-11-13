#pragma once


class Factory;

class SigmaTCLayer: public LayerObj {
protected:
    SigmaTCLayer() {}
    friend class Factory;
public:    
    SigmaTCLayer(const SigmaTCLayerC *_c) : c(_c) {}

    void print(std::ostream& str) const {
        str << "Hi, I am SigmaTCLayer, and I know nothing\n";
    }


private:
    shared_ptr<const SigmaTCLayerC> c;
};

