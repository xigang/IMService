/* *****************************************************************************
 * Author:         Xigang Wang
 * Email:          wangxigang2014@gmail.com
 * Last modfified: 2015-05-10 15:20
 * Filename:       mysql_connect.c
 * Note:           任何人可以任意复制代码并运用这些文档，当然包括你的商业用途, 但请遵循GPL
 * *****************************************************************************/
/*
 `********************************************************************
 **********************mysqlconnect.c*******************************
 */

#include "mysql_connect.h"

#define QUEUESIZE 3

/*  数据库连接池实现文件
 *  原理：先生成两个队列，一个用于存储空闲连接，一个用于存储正在使用连接；
 *      入口函数调用连接池函数，连接池函数查找空闲队列，有空闲的则函数返回一个连
 *      接句柄，如果没有则一直等待信号（该信号提示空闲队列从空到不空）；当获得空
 *      闲连接的句柄后，将该句柄插入到正在使用的队列中，再用这个句柄做数据库连接
 *      操作。当该连接使用完毕以后，从使用队列中删除，再插入到空闲队列中。
 ******************************************************************************
 *                            “mysqlpool.c"
 ******************************************************************************
 *
 */

//获取一个数据库连接
MYSQL * GetConnect()
{
	MYSQL *conn;
	char *server = "localhost";
	char *user = "root";
	char *password = "wangxigang";
	char *database = "im_instant_message";

	conn = mysql_init(NULL);
	while (!mysql_real_connect(conn, server, user, password, database, 0, NULL,
			0))
		;

	return conn;
}

int idIsExist(MYSQL *connect, char *id)
{
	MYSQL_RES *res;
	MYSQL_ROW sqlrow;
	char tmp[15];
	char sqlstate[100] = "select online from Users where id='";
	strcpy(tmp, id);
	strcat(sqlstate, tmp);
	strcat(sqlstate, "'");
	if (mysql_query(connect, sqlstate))
	{
		fprintf(stderr, "查id错误: %s \n", mysql_error(connect));
	}
	else
	{
		res = mysql_store_result(connect);
		if (res)
		{
			if (mysql_num_rows(res) > 0)
			{
				if ((sqlrow = mysql_fetch_row(res)))
				{
					if (strcmp(sqlrow[0], "0") == 0)
						return 1;
					else
						return 2;
				}
			}
			else
				return 0;
		}
		return -1;
	}
	return 0;
}

char *findIpbyId(MYSQL *connect, char *id)
{
	MYSQL_RES *res;
	MYSQL_ROW sqlrow;
	char sqlstate[100] =
			"select ip4 from Users where onlinestatus='1' and id='";
	strcat(sqlstate, id);
	strcat(sqlstate, "'");
	if (mysql_query(connect, sqlstate))
	{
		fprintf(stderr, "查IP错误: %s \n", mysql_error(connect));
	}
	else
	{
		res = mysql_store_result(connect);
		if (res)
		{
			if (mysql_num_rows(res) > 0)
			{
				if ((sqlrow = mysql_fetch_row(res)))
				{
					return sqlrow[0];
				}
			}
		}
	}
	return "0\0";
}

char *findNamebyId(MYSQL *connect, char *id)
{
	MYSQL_RES *res;
	MYSQL_ROW sqlrow;
	char sqlstate[100] = "select * from Users where id='";
	strcat(sqlstate, id);
	strcat(sqlstate, "'");
	printf("find sqlstate = %s\n", sqlstate);
	if (mysql_query(connect, sqlstate))
	{
		fprintf(stderr, "查用户名错误：%s \n", mysql_error(connect));
	}
	else
	{
		res = mysql_store_result(connect);
		if (res)
		{
			if (mysql_num_rows(res) > 0)
			{
				if ((sqlrow = mysql_fetch_row(res)))
				{
					return sqlrow[2];
				}
			}
		}
	}
	return "0\0";
}

int findsockfd(MYSQL *connect, char *id1)
{
	MYSQL_RES *res;
	MYSQL_ROW sqlrow;
	char sqlstate[100] = "select fd from Users where id='";
	strcat(sqlstate, id1);
	strcat(sqlstate, "'");
	if (mysql_query(connect, sqlstate))
	{
		fprintf(stderr, "查询错误：%s \n", mysql_error(connect));
	}
	else
	{
		res = mysql_store_result(connect);
		if (res)
		{
			if (mysql_num_rows(res) > 0)
			{
				if ((sqlrow = mysql_fetch_row(res)))
				{
					printf("此id:%s 的fd 是%d \n", id1, (int) sqlrow[0]);
					return (int) sqlrow[0];
				}
			}
		}
	}
	return -1;
}

//查询群中的ID
int findIdInGro(MYSQL *connect, char *group_id, char *id)
{
	MYSQL_RES *res;
	char sqlstate[100] = "select * from toGroId where group_id='";
	strcat(sqlstate, group_id);
	strcat(sqlstate, "' and id='");
	strcat(sqlstate, id);
	strcat(sqlstate, "'");
	if (mysql_query(connect, sqlstate))
	{
		fprintf(stderr, "查groupid 出错：%s \n", mysql_error(connect));
	}
	else
	{
		res = mysql_store_result(connect);
		if (res)
		{
			if (mysql_num_rows(res) > 0)
			{
				return 1;
			}
		}
	}
	return 0;
}

//查询群聊的聊天记录

char *findGroChatPath(MYSQL *connect, char *groupId)
{
	MYSQL_RES *res;
	MYSQL_ROW sqlrow;
	char sqlstate[100] = "select * from GroChatMesRecord where groupid='";
	strcat(sqlstate, groupId);
	strcat(sqlstate, "'");
	if (mysql_query(connect, sqlstate))
	{
		fprintf(stderr, "查询出错：%s \n", mysql_error(connect));
	}
	else
	{
		res = mysql_store_result(connect);
		if (res)
		{
			if (mysql_num_rows(res) > 0)
			{
				if ((sqlrow = mysql_fetch_row(res)))
				{
					printf("路径是：%s \n", sqlrow[1]);
					return sqlrow[1];
				}

			}
		}
	}
	return "0\0";
}

int insertGroChatPath(MYSQL *connect, char *group_id, char *path)
{
	char sqlstate[200] = "insert into GroChatMesRecord values ('";
	const char *temp = "','";
	strcat(sqlstate, group_id);
	strcat(sqlstate, temp);
	strcat(sqlstate, path);
	strcat(sqlstate, "')");
	if (mysql_query(connect, sqlstate))
	{
		fprintf(stderr, "插入群聊天路径出现错误：%s \n", mysql_error(connect));
	}
	return 1;
}

int insertAChatPath(MYSQL *connect, char *id1, char *id2, char *path)
{
	char sqlstate[200] = "insert into AChatMesRecord values ('";
	const char *temp = "','";
	strcat(sqlstate, id1);
	strcat(sqlstate, temp);
	strcat(sqlstate, id2);
	strcat(sqlstate, temp);
	strcat(sqlstate, path);
	strcat(sqlstate, "')");

	if (mysql_query(connect, sqlstate))
	{
		fprintf(stderr, "插入错误：%s \n", mysql_error(connect));
	}
	return 1;
}

//查询单对单的聊天记录
char *findAChatPath(MYSQL *connect, char *id1, char *id2)
{
	MYSQL_RES *res;
	MYSQL_ROW sqlrow;
	char sqlstate[100] = "select * from AChatMesRecord where id1='";
	const char *temp = "' and id2='";
	strcat(sqlstate, id1);
	strcat(sqlstate, temp);
	strcat(sqlstate, id2);
	strcat(sqlstate, "'");
	printf("sqlstate=%s\n", sqlstate);
	if (mysql_query(connect, sqlstate))
	{
		fprintf(stderr, "查询错误：%s \n", mysql_error(connect));
	}
	else
	{
		res = mysql_store_result(connect);
		if (res)
		{
			printf("查询结果行：%lu \n", (unsigned long) mysql_num_rows(res));
			if (mysql_num_rows(res) > 0)
			{
				if ((sqlrow = mysql_fetch_row(res)))
				{
					printf("路径是：%s \n", sqlrow[2]);
					return sqlrow[2];
				}
			}
		}

	}
	return "0\0";
}

//登录时验证用户名和密码
int findBynamepwd(char *employeeName, char *password, MYSQL *connect)
{
	MYSQL_RES *res;
	MYSQL_ROW sqlrow;
	char sqlstate[100] = "select * from Users where username='";
	const char * temp = "' and password='";
	strcat(sqlstate, employeeName);
	strcat(sqlstate, temp);
	strcat(sqlstate, password);
	strcat(sqlstate, "'");
	if (mysql_query(connect, sqlstate))
	{
		fprintf(stderr, "查询错误：%s \n", mysql_error(connect));
	}
	else
	{
		res = mysql_store_result(connect);
		if (res)
		{
			printf("查询结果行：%lu \n", (unsigned long) mysql_num_rows(res));
			if (mysql_num_rows(res) > 0)
			{
				if ((sqlrow = mysql_fetch_row(res)))
				{
					printf("shuju:%s\n", sqlrow[0]);
				}
				return 1;
			}
		}
		mysql_free_result(res);
	}
	return 0;
}

int updateOnlinestatus(MYSQL *connect, char * employeeid,
		const char * statustype)
{
	char sqlstate[100] = "update Users set online=";

	char *temp = " where id='";
	strcat(sqlstate, statustype);
	strcat(sqlstate, temp);
	strcat(sqlstate, employeeid);
	strcat(sqlstate, "'");
	printf("sqlstate=%s \n", sqlstate);
	if (mysql_query(connect, sqlstate))
	{
		fprintf(stderr, "状态ip更新错误：%s \n", mysql_error(connect));
	}
	else
	{
		if (((unsigned long) mysql_affected_rows(connect)) > 0)
		{
			printf("更新拉\n");
			printf("状态更新条数：%lu \n",
					(unsigned long) mysql_affected_rows(connect));
			return 1;
		}

	}
	return 0;
}

int updateIp(MYSQL *connect, char * employeeid, const char * ip)
{
	char sqlstate_ip[100] = "update Users set ip_addr='";
	char *temp1 = "' where id='";
	strcat(sqlstate_ip, ip);
	strcat(sqlstate_ip, temp1);
	strcat(sqlstate_ip, employeeid);
	strcat(sqlstate_ip, "'");
	printf("sqlstate_ip=%s \n", sqlstate_ip);
	if (mysql_query(connect, sqlstate_ip))
	{
		fprintf(stderr, "状态ip更新错误:%s \n", mysql_error(connect));
	}
	else
	{
		if (((unsigned long) mysql_affected_rows(connect)) > 0)
		{
			printf("状态更新条数：%lu \n",
					(unsigned long) mysql_affected_rows(connect));
			return 1;
		}

	}
	return 0;
}

int allKnoUsers(MYSQL *connect, s_user_info *userInfo)
{
	int j;

	MYSQL_RES *res;
	MYSQL_ROW sqlrow;
	char sqlstate[100] = "select * from Users";
	if (mysql_query(connect, sqlstate))
	{

		fprintf(stderr, "查id错误: %s \n", mysql_error(connect));
		return -1;
	}
	else
		res = mysql_store_result(connect);
	j = mysql_num_rows(res) - 1;

	while (j > 0)
	{
		sqlrow = mysql_fetch_row(res);
		sscanf(sqlrow[0], "%d", &userInfo->item_type);

		strcpy(userInfo->id, sqlrow[1]);
		strcpy(userInfo->name, sqlrow[2]);

		userInfo->ip_addr.s_addr = inet_addr(sqlrow[3]);

		sscanf(sqlrow[4], "%d", &userInfo->face);
		sscanf(sqlrow[5], "%d", (int *) &userInfo->online);

		j--;
		printf("j =====%d\n", j);
		printf("userinfo.item=%s\n", userInfo->name);
		printf("userinfo.face=%d\n", userInfo->face);
		printf("userinfo.ip=%d\n", userInfo->ip_addr.s_addr);
		userInfo++;
	}

	return 0;
}
