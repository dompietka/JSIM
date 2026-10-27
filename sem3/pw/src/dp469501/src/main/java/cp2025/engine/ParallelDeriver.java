package cp2025.engine;

import java.util.*;
import java.util.concurrent.Callable;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.stream.Collectors;

import cp2025.engine.Datalog.*;


public class ParallelDeriver implements AbstractDeriver {
    private final int numWorkers;
    private final ExecutorService executor;

    public ParallelDeriver(int numWorkers) { 
        this.numWorkers = numWorkers;
        this.executor = Executors.newFixedThreadPool(numWorkers);}

    @Override
    public Map<Atom, Boolean> derive(Program input, AbstractOracle oracle)
            throws InterruptedException {

        //potrzebne struktury wspolne dla watkow
        ConcurrentHashMap<Atom, Boolean> results = new ConcurrentHashMap<>(); 
        ConcurrentHashMap<Atom, Boolean> knownStatements = new ConcurrentHashMap<>();
        ConcurrentHashMap<Atom, ConcurrentLinkedQueue<Thread>> generalInProgressStatements = new ConcurrentHashMap<>();
        ConcurrentHashMap<Thread, Boolean> isCancelled = new ConcurrentHashMap<>();
        ConcurrentHashMap<Thread, ConcurrentLinkedQueue<Atom>> submited = new ConcurrentHashMap<>();
        ConcurrentHashMap<Predicate, ConcurrentLinkedQueue<Rule>> predicateToRules = //wypisac i sprawdzic czy dziala dobrze
        input.rules().stream().collect(
            Collectors.groupingBy(
            rule -> rule.head().predicate(),
            ConcurrentHashMap::new,               
            Collectors.toCollection(ConcurrentLinkedQueue::new)  
        )
    );

        
        List<Callable<Void>> tasks = new ArrayList<>();

        for (Atom atom : input.queries()) {
            tasks.add(() -> {

                submited.computeIfAbsent(Thread.currentThread(), t -> new ConcurrentLinkedQueue<>());
                isCancelled.put(Thread.currentThread(), false);
                ParallelDeriverState state = new ParallelDeriverState(input, oracle, knownStatements, generalInProgressStatements,
                     isCancelled, submited, predicateToRules);
                ParallelDeriverState.DerivationResult result = state.deriveStatement(atom);
                results.put(atom, result.derivable);
                return null;
            });
        }

        try{
            executor.invokeAll(tasks);
        }
        catch (InterruptedException e)
        {
            for (Thread t : isCancelled.keySet())
            {
                isCancelled.put(t, true);
                t.interrupt();
            }
        }
        finally{
            executor.shutdownNow();
        }
        return results;
    }
    
    public final class ParallelDeriverState { //dla kazdego zadania nowe, wiec tez tylko jeden watek ma dostep
        private final Program input;
        private final AbstractOracle oracle;

        ConcurrentHashMap<Atom, Boolean> knownStatements;
        ConcurrentHashMap<Atom, ConcurrentLinkedQueue<Thread>> generalInProgressStatements;
        ConcurrentHashMap<Thread, Boolean> isCancelled;
        ConcurrentHashMap<Thread, ConcurrentLinkedQueue<Atom>> submited;
        ConcurrentHashMap<Predicate, ConcurrentLinkedQueue<Rule>> predicateToRules;
        

        private final Set<Atom> inProgress = new HashSet<>();
        
        public ParallelDeriverState(Program input, AbstractOracle oracle, ConcurrentHashMap<Atom, Boolean> knownStatements,
         ConcurrentHashMap<Atom, ConcurrentLinkedQueue<Thread>> generalInProgressStatements, ConcurrentHashMap<Thread, Boolean> isCancelled,
         ConcurrentHashMap<Thread, ConcurrentLinkedQueue<Atom>> submited,
         ConcurrentHashMap<Predicate, ConcurrentLinkedQueue<Rule>> predicateToRules) {
            this.knownStatements = knownStatements;
            this.generalInProgressStatements = generalInProgressStatements;
            this.isCancelled = isCancelled;
            this.submited = submited;
            this.predicateToRules = predicateToRules;
            this.input = input;
            this.oracle = oracle;
        }

        private record DerivationResult(boolean derivable, Set<Atom> failedStatements) {}
        
        public DerivationResult deriveStatement(Atom goal) throws InterruptedException {
             // Check if we already know the result for this statement.
            if (knownStatements.containsKey(goal))
                return new DerivationResult(knownStatements.get(goal), Set.of());

           // veryifying if interuption happened, and was it cancelation or from another thread
            if (Thread.interrupted())
            {
                if (isCancelled.get(Thread.currentThread()) == true || submited.isEmpty())
                    throw new InterruptedException("Derivation process was interrupted.");

                for (Atom a : submited.getOrDefault(Thread.currentThread(), new ConcurrentLinkedQueue<>()))
                {
                    if (goal.equals(a)) {
                        submited.get(Thread.currentThread()).remove(a);
                        if (!submited.isEmpty()) Thread.currentThread().interrupt(); 
                        return new DerivationResult(knownStatements.get(a), Set.of());
                    }
                    else {
                        throw new InterruptedException("Something in progress got calculated.");
                    }

                }
            }

            // Check if the statement is calculatable
            if (oracle.isCalculatable(goal.predicate())) {
                try{
                    boolean result = oracle.calculate(goal);
                    knownStatements.put(goal, result); 
            
                    for (Thread t : generalInProgressStatements.getOrDefault(goal, new ConcurrentLinkedQueue<>()))
                    {
                        submited.computeIfAbsent(t, k -> new ConcurrentLinkedQueue<>()).add(goal);
                        t.interrupt();
                    }
                    return new DerivationResult(result, Set.of());
                }
                catch (InterruptedException e)
                {
                    if (isCancelled.get(Thread.currentThread()) == true || submited.isEmpty())
                    throw new InterruptedException("Derivation process was interrupted.");

                    for (Atom a : submited.getOrDefault(Thread.currentThread(), new ConcurrentLinkedQueue<>()))
                    {
                        if (goal.equals(a)) {
                            submited.get(Thread.currentThread()).remove(a);
                            if (!submited.isEmpty()) Thread.currentThread().interrupt(); 
                            return new DerivationResult(knownStatements.get(a), Set.of());
                        }
                        else {
                            throw new InterruptedException("Something in progress got calculated.");
                        }

                    }
                }
                
            }

            // Check for cycles, to avoid infinite loops.
            if (inProgress.contains(goal)) {
                // Return false but do not store the result (we may find a different derivation later).
                return new DerivationResult(false, Set.of(goal));
            }
            generalInProgressStatements.computeIfAbsent(goal, k -> new ConcurrentLinkedQueue<>()).add(Thread.currentThread());
            inProgress.add(goal);
            
            //if interupted here, we can just catch it in deriveNewStatement

            // Try to actually derive the statement using rules.
            
            try{
                DerivationResult result = deriveNewStatement(goal, predicateToRules);

                generalInProgressStatements.get(goal).remove(Thread.currentThread());
                inProgress.remove(goal);

                if (result.derivable) {
                    knownStatements.put(goal, true);
                    for (Thread t : generalInProgressStatements.getOrDefault(goal, new ConcurrentLinkedQueue<>()))
                    {
                        submited.computeIfAbsent(t, k -> new ConcurrentLinkedQueue<>()).add(goal);
                        t.interrupt();
                    }
                } else {
                    // We can only deduce non-derivability when there are no in-progress statements
                    // (at the top of the recursion).
                    if (inProgress.isEmpty())
                        for (Atom s : result.failedStatements){
                        knownStatements.put(s, false);
                        for (Thread t : generalInProgressStatements.getOrDefault(goal, new ConcurrentLinkedQueue<>()))
                        {
                            submited.computeIfAbsent(t, k -> new ConcurrentLinkedQueue<>()).add(goal);
                            t.interrupt();
                        }
                    }  
                }
                return result;
                
            }
            catch (InterruptedException e)
            {
                if (isCancelled.get(Thread.currentThread()) == true || submited.isEmpty())
                    throw new InterruptedException("Derivation process was interrupted.");

                for (Atom a : submited.getOrDefault(Thread.currentThread(), new ConcurrentLinkedQueue<>()))
                {
                    if (goal.equals(a)) {
                        submited.get(Thread.currentThread()).remove(a);
                        if (!submited.isEmpty()) Thread.currentThread().interrupt(); 
                        return new DerivationResult(knownStatements.get(a), Set.of());
                    }
                    else {
                        throw new InterruptedException("Something in progress got calculated.");
                    }

                }
            }
            return null;
        }

        //to zostaje wlasciwie bez zmian bo jest wywolywane tylko z jednego watku i dotyczy danych tylko jednego watku, oprocz predicateToRules
        private DerivationResult deriveNewStatement(Atom goal, ConcurrentHashMap<Predicate, ConcurrentLinkedQueue<Rule>> predicateToRules) throws InterruptedException {
            ConcurrentLinkedQueue<Rule> rules = predicateToRules.get(goal.predicate());
            if (rules == null) // czy jest jakakolwiek regula ktora pasuje 
                return new DerivationResult(false, Set.of(goal));

            Set<Atom> failedStatements = new HashSet<>();

            for (Rule rule : rules) {
                Optional<List<Atom>> partiallyAssignedBody = Unifier.unify(rule, goal);
                if (partiallyAssignedBody.isEmpty()) // czy glowe da sie dopasowac do celu
                    continue;

                
                List<Variable> variables = Datalog.getVariables(partiallyAssignedBody.get()); //sprawdza jakie zmienne sa w tym dopasowaniu
                FunctionGenerator<Variable, Constant> iterator = new FunctionGenerator<>(variables,
                        input.constants()); //generuje wszystkie mozliwe przypisania tych zmiennych do stalych
                for (Map<Variable, Constant> assignment : iterator) {
                    List<Atom> assignedBody = Unifier.applyAssignment(partiallyAssignedBody.get(),
                            assignment); // dla kazdego przypisania sprawdzamy czy cialo dziala
                    
                    try {
                        DerivationResult result = deriveBody(assignedBody);if (result.derivable)
                        return new DerivationResult(true, Set.of());
                        failedStatements.addAll(result.failedStatements);
                    } catch (InterruptedException e) {
                        if (isCancelled.get(Thread.currentThread()) == true || submited.isEmpty())
                        throw new InterruptedException("Derivation process was interrupted.");

                        for (Atom a : submited.getOrDefault(Thread.currentThread(), new ConcurrentLinkedQueue<>()))
                        {
                            if (goal.equals(a)) {
                                submited.get(Thread.currentThread()).remove(a);
                                if (!submited.isEmpty()) Thread.currentThread().interrupt(); 
                                return new DerivationResult(knownStatements.get(a), Set.of());
                            }
                            else {
                                throw new InterruptedException("Something in progress got calculated.");
                            }

                        }
                                
                    }
                    
                }
            }

            failedStatements.add(goal);
            return new DerivationResult(false, failedStatements);
        }

        private DerivationResult deriveBody(List<Atom> body) throws InterruptedException {
            for (Atom statement : body) {
                try {
                    DerivationResult result = deriveStatement(statement);
                    if (!result.derivable)
                        return new DerivationResult(false, result.failedStatements);
                }
                catch (InterruptedException e)
                {
                    if (isCancelled.get(Thread.currentThread()) == true || submited.isEmpty())
                        throw new InterruptedException("Derivation process was interrupted.");

                    for (Atom a : submited.getOrDefault(Thread.currentThread(), new ConcurrentLinkedQueue<>()))
                    {
                        if (statement.equals(a)) {
                            submited.get(Thread.currentThread()).remove(a);
                            if (!submited.isEmpty()) Thread.currentThread().interrupt(); 
                            return new DerivationResult(knownStatements.get(a), Set.of());
                        }
                        else {
                            throw new InterruptedException("Something in progress got calculated.");
                        }

                    }
                }
               
            }
            return new DerivationResult(true, Set.of());
        }
            
    }

}
