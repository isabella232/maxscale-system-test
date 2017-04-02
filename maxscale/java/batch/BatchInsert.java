package maxscale.java.batch;

import maxscale.java.MaxScaleConfiguration;
import maxscale.java.MaxScaleConnection;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.Statement;

public class BatchInsert {

    public static void main(String[] args) {
        try {

            MaxScaleConfiguration config = new MaxScaleConfiguration("batchinsert");
            MaxScaleConnection maxscale = new MaxScaleConnection("useBatchMultiSendNumber=500");

            Connection connection = maxscale.getConnRw();
            Statement stmt = connection.createStatement();
            stmt.execute("CREATE OR REPLACE TABLE tt (d int)");

            for (int i = 0; i < 300; i++) {
                switch (i % 4) {
                case 0:
                    stmt.addBatch("SET @test2='aaa'");
                    break;

                case 1:
                    stmt.addBatch("INSERT INTO tt(d) VALUES (1)");
                    break;

                case 2:
                    stmt.addBatch("SELECT * FROM tt");
                    break;

                case 3:
                    stmt.addBatch("SET @test2=(SELECT SLEEP 0.1)");
                    break;
                }
            }

            stmt.executeBatch();

            PreparedStatement ps = connection.prepareStatement("SELECT * FROM tt WHERE d <> ?");

            for (int i = 0; i < 300; i++) {
                ps.setInt(1, i);
                ps.addBatch();
            }

            ps.executeBatch();

        } catch (Exception e) {
            System.out.println("Error: " + e.getMessage());
            System.exit(1);
        }
    }
}
