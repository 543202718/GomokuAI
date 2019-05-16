package gomokugui;

import java.io.InputStream;
import java.util.NoSuchElementException;
import java.util.Scanner;

/**
 *
 * @author Wang
 */
public class GomokuGUI {

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        MainFrame.getInstance().setVisible(true);
    }
    
}

class ProcessListener extends Thread implements Runnable{
    Process process;  
    ProcessListener(Process p){
        process=p;
    }
    @Override
    public void run() {
        InputStream pInputStream =process.getInputStream();
        Scanner sc=new Scanner(pInputStream);
        try{
            while(true){
                String s=sc.next();
                if (s.contains("AI")){
                    s=sc.next();
                    MainFrame.getInstance().gameOver(!s.contains("Win"));
                }
                else {
                    int x=Integer.parseInt(s);
                    int y=sc.nextInt();
                    System.out.println("AI: "+x+" "+y);
                    MainFrame.getInstance().putBlackChess(x, y); 
                }     
            }
        }catch(NoSuchElementException ex){
            System.out.println("Game over!");
            this.interrupt();
        }
    }
    
}