#include<iostream>
#include<soci/soci.h>



int main()
{
    try
    {
        int count = 0;
        // connect database
        // method 1
        soci::session conn("mysql", "host=127.0.0.1 port=3306 db=camera_guard_business user=root password=root");
        // method 1
        //soci::session conn;
        //conn.open("mysql://host=127.0.0.1 port=3306 db=MyQQ  user=root password=root");
        conn << "select count(*) from ft_camera"
            , soci::into(count);
        std::cout << count << std::endl;
    }
    catch (const std::exception& e)
    {
        std::cout << "Operate database exception: " << e.what();
    }

    system("pause");
    return 0;
}