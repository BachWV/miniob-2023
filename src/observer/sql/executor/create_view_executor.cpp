#include "sql/executor/create_view_executor.h"

#include "event/sql_event.h"
#include "event/session_event.h"
#include "storage/db/db.h"
#include "session/session.h"
#include "sql/stmt/view_stmt.h"
#include "sql/view/view.h"

RC CreateViewExecutor::execute(SQLStageEvent *sql_event)
{
    Stmt *stmt = sql_event->stmt();
    assert(stmt->type() == StmtType::CREATE_VIEW);
    ViewStmt *view_stmt = dynamic_cast<ViewStmt*>(stmt);
    Session *session = sql_event->session_event()->session();
    return session->get_current_db()->create_view(view_stmt->owns_view());
}