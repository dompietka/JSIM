package totolotek;

import org.junit.jupiter.api.Test;
import totolotek.system.NumerPorzadkowy;

class NumerPorzadkowyTest {
    @Test
    void testWypisywania(){
        NumerPorzadkowy np = new NumerPorzadkowy(2137);
        System.out.println(np.get());
    }
}